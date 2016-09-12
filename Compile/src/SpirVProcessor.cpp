/*
 * Copyright 2016 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SpirVProcessor.h"
#include <MSL/Compile/CompiledResult.h>
#include <SPIRV/spirv.hpp>
#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace msl
{

namespace
{

static const unsigned int firstInstruction = 5;
static const unsigned int unknownLength = (unsigned int)-1;
static const std::uint32_t unset = (std::uint32_t)-1;;

static std::uint32_t typeSizes[] =
{
	// Scalars and vectors
	sizeof(float),    // Float
	sizeof(float)*2,  // Vec2
	sizeof(float)*3,  // Vec3
	sizeof(float)*4,  // Vec4
	sizeof(double),   // Double
	sizeof(double)*2, // DVec2
	sizeof(double)*3, // DVec3
	sizeof(double)*4, // DVec4
	sizeof(int),      // Int
	sizeof(int)*2,    // IVec2
	sizeof(int)*3,    // IVec3
	sizeof(int)*4,    // IVec4
	sizeof(int),      // UInt
	sizeof(int)*2,    // UVec2
	sizeof(int)*3,    // UVec3
	sizeof(int)*4,    // UVec4
	sizeof(int),      // Bool
	sizeof(int)*2,    // BVec2
	sizeof(int)*3,    // BVec3
	sizeof(int)*4,    // BVec4

	// Matrices
	sizeof(float)*4*2,  // Mat2
	sizeof(float)*4*3,  // Mat3
	sizeof(float)*4*2,  // Mat4
	sizeof(float)*4*3,  // Mat2x3
	sizeof(float)*4*4,  // Mat2x4
	sizeof(float)*4*2,  // Mat3x2
	sizeof(float)*4*4,  // Mat3x4
	sizeof(float)*4*2,  // Mat4x2
	sizeof(float)*4*3,  // Mat4x3
	sizeof(double)*2*2, // DMat2
	sizeof(double)*4*2, // DMat3
	sizeof(double)*4*4, // DMat4
	sizeof(double)*2*3, // DMat2x3
	sizeof(double)*2*4, // DMat2x4
	sizeof(double)*4*2, // DMat3x2
	sizeof(double)*4*4, // DMat3x4
	sizeof(double)*4*2, // DMat4x2
	sizeof(double)*4*3, // DMat4x3
};

struct SpirArrayInfo
{
	std::uint32_t type;
	unsigned int length;
};

struct IntermediateData
{
	// Names
	std::unordered_map<std::uint32_t, std::string> names;
	std::unordered_map<std::uint32_t, std::vector<std::string>> memberNames;

	// Type info
	std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> structTypes;
	std::unordered_map<std::uint32_t, Type> types;
	std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> memberOffsets;
	std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> matrixStrides;
	std::unordered_map<std::uint32_t, std::uint32_t> intConstants;
	std::unordered_map<std::uint32_t, SpirArrayInfo> arrayTypes;
	std::unordered_map<std::uint32_t, std::uint32_t> arrayStrides;
	std::unordered_set<std::uint32_t> uniformBlocks;
	std::unordered_set<std::uint32_t> uniformBuffers;

	// Metadata
	std::unordered_map<std::uint32_t, std::uint32_t> descriptorSets;
	std::unordered_map<std::uint32_t, std::uint32_t> bindings;
	std::unordered_map<std::uint32_t, std::uint32_t> locations;

	// Variable declarations
	// Make these ordered (except for pointers) so they will be consistent across runs.
	std::unordered_map<std::uint32_t, std::uint32_t> pointers;
	std::map<std::uint32_t, std::uint32_t> uniformVars;
	std::map<std::uint32_t, std::uint32_t> inputVars;
	std::map<std::uint32_t, std::uint32_t> outputVars;
	std::map<std::uint32_t, std::uint32_t> imageVars;
	std::pair<std::uint32_t, std::uint32_t> pushConstantPointer = std::make_pair(unset, unset);
};

spv::Op getOp(std::uint32_t value)
{
	return static_cast<spv::Op>(value & spv::OpCodeMask);
}

unsigned int getWordCount(std::uint32_t value)
{
	return value >> spv::WordCountShift;
}

std::string readString(std::vector<char>& tempBuffer,
	const std::vector<std::uint32_t>& spirv, std::size_t start, std::size_t wordCount,
	std::size_t offset)
{
	tempBuffer.clear();
	std::size_t max = start + wordCount;
	assert(max < spirv.size());
	for (std::size_t i = offset; i < wordCount; ++i)
	{
		std::uint32_t chars = spirv[start + i];
		if (chars == 0)
		{
			tempBuffer.push_back(0);
			break;
		}
		else
		{
			// 4 characters are stuffed into each int.
			tempBuffer.push_back(static_cast<char>(chars & 0xFF));
			tempBuffer.push_back(static_cast<char>((chars >> 8) & 0xFF));
			tempBuffer.push_back(static_cast<char>((chars >> 16) & 0xFF));
			tempBuffer.push_back(static_cast<char>((chars >> 24) & 0xFF));
		}
	}

	return tempBuffer.data();
}

void readVector(IntermediateData& data, const std::vector<std::uint32_t>& spirv, std::size_t i,
	std::uint32_t wordCount)
{
	assert(wordCount == 4);
	std::uint32_t id = spirv[i + 1];
	std::uint32_t typeId = spirv[i + 2];
	std::uint32_t length = spirv[i + 3];
	auto foundType = data.types.find(typeId);
	assert(foundType != data.types.end());
	switch (foundType->second)
	{
		case Type::Bool:
			switch (length)
			{
				case 2:
					data.types[id] = Type::BVec2;
					break;
				case 3:
					data.types[id] = Type::BVec3;
					break;
				case 4:
					data.types[id] = Type::BVec4;
					break;
				default:
					assert(false);
					break;
			}
			break;
		case Type::Int:
			switch (length)
			{
				case 2:
					data.types[id] = Type::IVec2;
					break;
				case 3:
					data.types[id] = Type::IVec3;
					break;
				case 4:
					data.types[id] = Type::IVec4;
					break;
				default:
					assert(false);
					break;
			}
			break;
		case Type::UInt:
			switch (length)
			{
				case 2:
					data.types[id] = Type::UVec2;
					break;
				case 3:
					data.types[id] = Type::UVec3;
					break;
				case 4:
					data.types[id] = Type::UVec4;
					break;
				default:
					assert(false);
					break;
			}
			break;
		case Type::Float:
			switch (length)
			{
				case 2:
					data.types[id] = Type::Vec2;
					break;
				case 3:
					data.types[id] = Type::Vec3;
					break;
				case 4:
					data.types[id] = Type::Vec4;
					break;
				default:
					assert(false);
					break;
			}
			break;
		case Type::Double:
			switch (length)
			{
				case 2:
					data.types[id] = Type::DVec2;
					break;
				case 3:
					data.types[id] = Type::DVec3;
					break;
				case 4:
					data.types[id] = Type::DVec4;
					break;
				default:
					assert(false);
					break;
			}
			break;
		default:
			assert(false);
			break;
	}
}

void readMatrix(IntermediateData& data, const std::vector<std::uint32_t>& spirv, std::size_t i,
	std::uint32_t wordCount)
{
	assert(wordCount == 4);
	std::uint32_t id = spirv[i + 1];
	std::uint32_t typeId = spirv[i + 2];
	std::uint32_t length = spirv[i + 3];
	auto foundType = data.types.find(typeId);
	assert(foundType != data.types.end());
	switch (foundType->second)
	{
		case Type::Vec2:
			switch (length)
			{
				case 2:
					data.types[id] = Type::Mat2;
					break;
				case 3:
					data.types[id] = Type::Mat2x3;
					break;
				case 4:
					data.types[id] = Type::Mat2x4;
					break;
				default:
					assert(false);
			}
		case Type::Vec3:
			switch (length)
			{
				case 2:
					data.types[id] = Type::Mat3x2;
					break;
				case 3:
					data.types[id] = Type::Mat3;
					break;
				case 4:
					data.types[id] = Type::Mat3x4;
					break;
				default:
					assert(false);
			}
		case Type::Vec4:
			switch (length)
			{
				case 2:
					data.types[id] = Type::Mat4x2;
					break;
				case 3:
					data.types[id] = Type::Mat4x3;
					break;
				case 4:
					data.types[id] = Type::Mat4;
					break;
				default:
					assert(false);
			}
		case Type::DVec2:
			switch (length)
			{
				case 2:
					data.types[id] = Type::DMat2;
					break;
				case 3:
					data.types[id] = Type::DMat2x3;
					break;
				case 4:
					data.types[id] = Type::DMat2x4;
					break;
				default:
					assert(false);
			}
		case Type::DVec3:
			switch (length)
			{
				case 2:
					data.types[id] = Type::DMat3x2;
					break;
				case 3:
					data.types[id] = Type::DMat3;
					break;
				case 4:
					data.types[id] = Type::DMat3x4;
					break;
				default:
					assert(false);
			}
		case Type::DVec4:
			switch (length)
			{
				case 2:
					data.types[id] = Type::DMat4x2;
					break;
				case 3:
					data.types[id] = Type::DMat4x3;
					break;
				case 4:
					data.types[id] = Type::DMat4;
					break;
				default:
					assert(false);
			}
		default:
			assert(false);
			break;
	}
}

void readImage(IntermediateData& data, const std::vector<std::uint32_t>& spirv, std::size_t i,
	std::uint32_t wordCount)
{
	assert(wordCount >= 8);
	std::uint32_t id = spirv[i + 1];
	std::uint32_t typeId = spirv[i + 2];
	std::uint32_t dimension = spirv[i + 3];
	std::uint32_t depth = spirv[i + 4];
	std::uint32_t array = spirv[i + 5];
	std::uint32_t ms = spirv[i + 6];
	std::uint32_t sampled = spirv[i + 7];
	switch (dimension)
	{
		case spv::Dim1D:
		{
			assert(!ms);
			auto foundType = data.types.find(typeId);
			assert(foundType != data.types.end());
			switch (foundType->second)
			{
				case Type::Float:
					if (sampled)
					{
						if (depth == 1)
						{
							if (array)
								data.types[id] = Type::Sampler1DArrayShadow;
							else
								data.types[id] = Type::Sampler1DShadow;
						}
						else
						{
							if (array)
								data.types[id] = Type::Sampler1DArray;
							else
								data.types[id] = Type::Sampler1D;
						}
					}
					else
					{
						if (array)
							data.types[id] = Type::Image1DArray;
						else
							data.types[id] = Type::Image1D;
					}
					break;
				case Type::Int:
					if (sampled)
					{
						if (array)
							data.types[id] = Type::ISampler1DArray;
						else
							data.types[id] = Type::ISampler1D;
					}
					else
					{
						if (array)
							data.types[id] = Type::IImage1DArray;
						else
							data.types[id] = Type::IImage1D;
					}
					break;
				case Type::UInt:
					if (sampled)
					{
						if (array)
							data.types[id] = Type::USampler1DArray;
						else
							data.types[id] = Type::USampler1D;
					}
					else
					{
						if (array)
							data.types[id] = Type::UImage1DArray;
						else
							data.types[id] = Type::UImage1D;
					}
					break;
				default:
					assert(false);
			}
			break;
		}
		case spv::Dim2D:
		{
			auto foundType = data.types.find(typeId);
			assert(foundType != data.types.end());
			switch (foundType->second)
			{
				case Type::Float:
					if (sampled)
					{
						if (depth == 1)
						{
							assert(!ms);
							if (array)
								data.types[id] = Type::Sampler2DArrayShadow;
							else
								data.types[id] = Type::Sampler2DShadow;
						}
						else
						{
							if (ms)
							{
								if (array)
									data.types[id] = Type::Sampler2DMSArray;
								else
									data.types[id] = Type::Sampler2DMS;
							}
							else
							{
								if (array)
									data.types[id] = Type::Sampler2DArray;
								else
									data.types[id] = Type::Sampler2D;
							}
						}
					}
					else
					{
						if (ms)
						{
							if (array)
								data.types[id] = Type::Image2DMSArray;
							else
								data.types[id] = Type::Image2DMS;
						}
						else
						{
							if (array)
								data.types[id] = Type::Image2DArray;
							else
								data.types[id] = Type::Image2D;
						}
					}
					break;
				case Type::Int:
					if (sampled)
					{
						if (ms)
						{
							if (array)
								data.types[id] = Type::ISampler2DMSArray;
							else
								data.types[id] = Type::ISampler2DMS;
						}
						else
						{
							if (array)
								data.types[id] = Type::ISampler2DArray;
							else
								data.types[id] = Type::ISampler2D;
						}
					}
					else
					{
						if (ms)
						{
							if (array)
								data.types[id] = Type::IImage2DMSArray;
							else
								data.types[id] = Type::IImage2DMS;
						}
						else
						{
							if (array)
								data.types[id] = Type::IImage2DArray;
							else
								data.types[id] = Type::IImage2D;
						}
					}
					break;
				case Type::UInt:
					if (sampled)
					{
						if (ms)
						{
							if (array)
								data.types[id] = Type::USampler2DMSArray;
							else
								data.types[id] = Type::USampler2DMS;
						}
						else
						{
							if (array)
								data.types[id] = Type::USampler2DArray;
							else
								data.types[id] = Type::USampler2D;
						}
					}
					else
					{
						if (ms)
						{
							if (array)
								data.types[id] = Type::UImage2DMSArray;
							else
								data.types[id] = Type::UImage2DMS;
						}
						else
						{
							if (array)
								data.types[id] = Type::UImage2DArray;
							else
								data.types[id] = Type::UImage2D;
						}
					}
					break;
				default:
					assert(false);
					break;
			}
			break;
		}
		case spv::Dim3D:
		{
			assert(!ms);
			assert(!array);
			auto foundType = data.types.find(typeId);
			assert(foundType != data.types.end());
			switch (foundType->second)
			{
				case Type::Float:
					if (sampled)
						data.types[id] = Type::Sampler3D;
					else
						data.types[id] = Type::Image3D;
					break;
				case Type::Int:
					if (sampled)
						data.types[id] = Type::ISampler3D;
					else
						data.types[id] = Type::IImage3D;
					break;
				case Type::UInt:
					if (sampled)
						data.types[id] = Type::USampler3D;
					else
						data.types[id] = Type::UImage3D;
					break;
				default:
					assert(false);
					break;
			}
			break;
		}
		case spv::DimCube:
		{
			assert(!ms);
			assert(!array);
			auto foundType = data.types.find(typeId);
			assert(foundType != data.types.end());
			switch (foundType->second)
			{
				case Type::Float:
					if (sampled)
					{
						if (depth == 1)
							data.types[id] = Type::SamplerCubeShadow;
						else
							data.types[id] = Type::SamplerCube;
					}
					else
						data.types[id] = Type::ImageCube;
					break;
				case Type::Int:
					if (sampled)
						data.types[id] = Type::ISamplerCube;
					else
						data.types[id] = Type::IImageCube;
					break;
				case Type::UInt:
					if (sampled)
						data.types[id] = Type::USamplerCube;
					else
						data.types[id] = Type::UImageCube;
					break;
				default:
					assert(false);
					break;
			}
			break;
		}
		case spv::DimRect:
		{
			assert(!ms);
			assert(!array);
			auto foundType = data.types.find(typeId);
			assert(foundType != data.types.end());
			switch (foundType->second)
			{
				case Type::Float:
					if (sampled)
					{
						if (depth == 1)
							data.types[id] = Type::Sampler2DRectShadow;
						else
							data.types[id] = Type::Sampler2DRect;
					}
					else
						data.types[id] = Type::Image2DRect;
					break;
				case Type::Int:
					if (sampled)
						data.types[id] = Type::ISampler2DRect;
					else
						data.types[id] = Type::IImage2DRect;
					break;
				case Type::UInt:
					if (sampled)
						data.types[id] = Type::USampler2DRect;
					else
						data.types[id] = Type::UImage2DRect;
					break;
				default:
					assert(false);
					break;
			}
			break;
		}
		case spv::DimBuffer:
		{
			assert(!ms);
			assert(!array);
			if (sampled)
				data.types[id] = Type::SamplerBuffer;
			else
				data.types[id] = Type::ImageBuffer;
			break;
		}
		case spv::DimSubpassData:
		{
			assert(!array);
			assert(!sampled);
			auto foundType = data.types.find(typeId);
			assert(foundType != data.types.end());
			switch (foundType->second)
			{
				case Type::Float:
					if (ms)
						data.types[id] = Type::SubpassInputMS;
					else
						data.types[id] = Type::SubpassInput;
					break;
				case Type::Int:
					if (ms)
						data.types[id] = Type::ISubpassInputMS;
					else
						data.types[id] = Type::ISubpassInput;
					break;
				case Type::UInt:
					if (ms)
						data.types[id] = Type::USubpassInputMS;
					else
						data.types[id] = Type::USubpassInput;
					break;
				default:
					assert(false);
					break;
			}
		}
		default:
			assert(false);
			break;
	}
}

bool isMatrix(Type type)
{
	switch (type)
	{
		case Type::Mat2:
		case Type::Mat3:
		case Type::Mat4:
		case Type::Mat2x3:
		case Type::Mat2x4:
		case Type::Mat3x2:
		case Type::Mat3x4:
		case Type::Mat4x2:
		case Type::Mat4x3:
		case Type::DMat2:
		case Type::DMat3:
		case Type::DMat4:
		case Type::DMat2x3:
		case Type::DMat2x4:
		case Type::DMat3x2:
		case Type::DMat3x4:
		case Type::DMat4x2:
		case Type::DMat4x3:
			return true;
		default:
			return false;
	}
}

std::uint32_t getTypeSize(const SpirVProcessor& processor, Type type, std::uint32_t structIndex)
{
	if (type == Type::Struct)
	{
		assert(structIndex < processor.structs.size());
		return processor.structs[structIndex].size;
	}
	else
	{
		unsigned int typeIndex = static_cast<unsigned int>(type);
		assert(typeIndex < sizeof(typeSizes)/sizeof(*typeSizes));
		return typeSizes[typeIndex];
	}
}

Type getType(std::vector<ArrayInfo>& arrayElements, std::uint32_t& structIndex,
	SpirVProcessor& processor, const IntermediateData& data, std::uint32_t typeId)
{
	structIndex = unknown;

	// Resolve arrays first.
	auto foundArray = data.arrayTypes.find(typeId);
	while (foundArray != data.arrayTypes.end())
	{
		arrayElements.emplace_back();
		arrayElements.back().length = foundArray->second.length;

		auto foundArrayStride = data.arrayStrides.find(typeId);
		assert(foundArrayStride != data.arrayStrides.end());
		arrayElements.back().stride = foundArrayStride->second;

		typeId = foundArray->second.type;
		foundArray = data.arrayTypes.find(typeId);
	}

	// Check if it's a struct.
	auto foundStruct = data.structTypes.find(typeId);
	if (foundStruct == data.structTypes.end())
	{
		auto foundType = data.types.find(typeId);
		assert(foundType != data.types.end());
		return foundType->second;
	}

	// Get the index of the struct.
	auto foundStructId = std::find(processor.structIds.begin(), processor.structIds.end(), typeId);
	if (foundStructId != processor.structIds.end())
	{
		structIndex = static_cast<std::uint32_t>(foundStructId - processor.structIds.begin());
		return Type::Struct;
	}

	// Haven't encountered this struct before; add it.
	// Get the name.
	auto foundStructName = data.names.find(typeId);
	assert(foundStructName != data.names.end());
	Struct newStruct;
	newStruct.name = foundStructName->second;

	// Get the member info.
	auto foundMemberNames = data.memberNames.find(typeId);
	assert(foundMemberNames != data.memberNames.end());
	assert(foundMemberNames->second.size() == foundStruct->second.size());

	auto foundMemberOffsets = data.memberOffsets.find(typeId);
	assert(foundMemberOffsets != data.memberOffsets.end());
	assert(foundMemberOffsets->second.size() == foundStruct->second.size());

	auto foundMatrixStrides = data.matrixStrides.find(typeId);

	newStruct.size = 0;
	newStruct.members.resize(foundStruct->second.size());
	for (std::size_t i = 0; i < foundStruct->second.size(); ++i)
	{
		std::uint32_t memberTypeId = foundStruct->second[i];
		StructMember& member = newStruct.members[i];
		member.name = foundMemberNames->second[i];
		member.offset = foundMemberOffsets->second[i];
		member.type = getType(member.arrayElements, member.structIndex, processor, data,
			memberTypeId);

		// Get the size of the member.
		if (!member.arrayElements.empty())
		{
			// If an array, the size is the stride times the number of elements.
			foundArray = data.arrayTypes.find(memberTypeId);
			assert(foundArray != data.arrayTypes.end());
			if (foundArray->second.length == unknown)
				member.size = unknown;
			else
			{
				auto foundArrayStride = data.arrayStrides.find(memberTypeId);
				assert(foundArrayStride != data.arrayStrides.end());
				member.size = foundArrayStride->second*foundArray->second.length;
			}
		}
		else if (isMatrix(member.type))
		{
			// Matrices have their own strides stored with the struct.
			assert(foundMatrixStrides != data.matrixStrides.end());
			assert(i < foundMatrixStrides->second.size());
			assert(foundMatrixStrides->second[i] != unknown);
			member.size = foundMatrixStrides->second[i];
		}
		else
			member.size = getTypeSize(processor, member.type, member.structIndex);
	}

	// Get the struct size based on the last member.
	// If the size of the last member is unknown (due to an unsized array), it's based on the offset
	// of the last element.
	if (!newStruct.members.empty())
	{
		const StructMember& lastMember = newStruct.members.back();
		newStruct.size = lastMember.offset;
		if (lastMember.size != unknown)
			newStruct.size += lastMember.size;

		// Must be a multiple of vec4.
		const unsigned int minAlignment = sizeof(float)*4;
		newStruct.size = ((newStruct.size + minAlignment - 1)/minAlignment)*minAlignment;
	}

	assert(processor.structs.size() == processor.structIds.size());
	processor.structs.push_back(std::move(newStruct));
	processor.structIds.push_back(typeId);
	structIndex = static_cast<std::uint32_t>(processor.structs.size() - 1);
	return Type::Struct;
}

void addUniforms(SpirVProcessor& processor, const IntermediateData& data)
{
	processor.uniforms.resize(data.uniformVars.size());
	processor.uniformIds.resize(data.uniformVars.size());
	std::size_t i = 0;
	for (const std::pair<std::uint32_t, std::uint32_t>& uniformIndices: data.uniformVars)
	{
		processor.uniformIds[i] = uniformIndices.first;

		auto foundPointer = data.pointers.find(uniformIndices.first);
		assert(foundPointer != data.pointers.end());
		std::uint32_t typeId = foundPointer->second;

		Uniform& uniform = processor.uniforms[i];
		uniform.type = getType(uniform.arrayElements, uniform.structIndex, processor, data, typeId);
		if (uniform.type == Type::Struct)
			uniform.name = processor.structs[uniform.structIndex].name;
		else
		{
			auto foundName = data.names.find(uniformIndices.first);
			assert(foundName != data.names.end());
			uniform.name = foundName->second;
		}

		if (data.uniformBlocks.find(uniformIndices.first) != data.uniformBlocks.end())
			uniform.uniformType = UniformType::Block;
		else
		{
			assert(data.uniformBuffers.find(uniformIndices.first) != data.uniformBuffers.end());
			uniform.uniformType = UniformType::BlockBuffer;
		}

		auto foundDescriptorSet = data.descriptorSets.find(uniformIndices.first);
		if (foundDescriptorSet == data.descriptorSets.end())
			uniform.descriptorSet = unknown;
		else
			uniform.descriptorSet = foundDescriptorSet->second;

		auto foundBinding = data.bindings.find(uniformIndices.first);
		if (foundBinding == data.bindings.end())
			uniform.binding = unknown;
		else
			uniform.binding = foundBinding->second;

		uniform.samplerIndex = unknown;
	}
}

} // namespace

SpirVProcessor::SpirVProcessor(const std::vector<std::uint32_t>& spirv)
{
	assert(spirv[0] == spv::MagicNumber);
	assert(spirv[1] == spv::Version);
	std::vector<char> tempBuffer;

	IntermediateData data;

	// Grab the metadata we want out of the SPIR-V.
	bool done = false;
	for (std::size_t i = firstInstruction; i < spirv.size() && !done;)
	{
		spv::Op op = getOp(spirv[i]);
		unsigned int wordCount = getWordCount(spirv[i]);
		assert(wordCount > 0 && wordCount + i <= spirv.size());
		switch (op)
		{
			// Extract names.
			case spv::OpName:
			{
				assert(wordCount >= 3);
				std::uint32_t id = spirv[i + 1];
				data.names[id] = readString(tempBuffer, spirv, i, wordCount, 2);
				break;
			}
			case spv::OpMemberName:
			{
				assert(wordCount >= 4);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t member = spirv[i + 2];
				std::vector<std::string>& thisMemberNames = data.memberNames[id];
				if (thisMemberNames.size() <= member)
					thisMemberNames.resize(member + 1);
				thisMemberNames[member] = readString(tempBuffer, spirv, i, wordCount, 3);
				break;
			}

			// Extract decorations that we care about.
			case spv::OpDecorate:
			{
				assert(wordCount >= 3);
				std::uint32_t id = spirv[i + 1];
				switch (spirv[i + 2])
				{
					case spv::DecorationDescriptorSet:
						assert(wordCount == 5);
						data.descriptorSets[id] = spirv[i + 4];
						break;
					case spv::DecorationBinding:
						assert(wordCount == 5);
						data.bindings[id] = spirv[i + 4];
						break;
					case spv::DecorationLocation:
						assert(wordCount == 5);
						data.locations[id] = spirv[i + 4];
						break;
					case spv::DecorationArrayStride:
						assert(wordCount == 5);
						data.arrayStrides[id] = spirv[i + 4];
						break;
					case spv::DecorationBlock:
						assert(wordCount == 4);
						data.uniformBlocks.insert(id);
						break;
					case spv::DecorationBufferBlock:
						assert(wordCount == 5);
						data.uniformBuffers.insert(id);
						break;
				}
				break;
			}
			case spv::OpMemberDecorate:
			{
				assert(wordCount >= 4);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t member = spirv[i + 2];
				switch (spirv[i + 3])
				{
					case spv::DecorationOffset:
					{
						assert(wordCount == 5);
						std::vector<std::uint32_t>& thisMemberOffsets = data.memberOffsets[id];
						if (thisMemberOffsets.size() <= member)
							thisMemberOffsets.resize(member + 1, unset);
						thisMemberOffsets[member] = spirv[i + 4];
						break;
					}
					case spv::DecorationMatrixStride:
					{
						assert(wordCount == 5);
						std::vector<std::uint32_t>& thisMemberStrides = data.matrixStrides[id];
						if (thisMemberStrides.size() <= member)
							thisMemberStrides.resize(member + 1, unset);
						thisMemberStrides[member] = spirv[i + 4];
						break;
					}
				}
				break;
			}

			// Extract integer contsnts.
			case spv::OpConstant:
			{
				assert(wordCount > 3);
				std::uint32_t typeId = spirv[i + 1];
				std::uint32_t id = spirv[i + 2];
				auto foundType = data.types.find(typeId);
				assert(foundType != data.types.end());
				switch (foundType->second)
				{
					case Type::Int:
					case Type::UInt:
						data.intConstants[id] = spirv[i + 3];
						break;
					default:
						break;
				}
				break;
			}

			// Extract type declarations.
			case spv::OpTypeBool:
				assert(wordCount == 2);
				data.types[spirv[i + 1]] = Type::Bool;
				break;
			case spv::OpTypeInt:
			{
				assert(wordCount == 4);
				assert(spirv[i + 2] == 32);
				std::uint32_t id = spirv[i + 1];
				if (spirv[i + 3])
					data.types[id] = Type::Int;
				else
					data.types[id] = Type::UInt;
				break;
			}
			case spv::OpTypeFloat:
			{
				assert(wordCount == 3);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t width = spirv[i + 2];
				assert(width == 32 || width == 64);
				if (width == 64)
					data.types[id] = Type::Double;
				else
					data.types[id] = Type::Float;
				break;
			}
			case spv::OpTypeVector:
				readVector(data, spirv, i, wordCount);
				break;
			case spv::OpTypeMatrix:
				readMatrix(data, spirv, i, wordCount);
				break;
			case spv::OpTypeArray:
			{
				assert(wordCount == 4);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t type = spirv[i + 2];
				std::uint32_t constantId = spirv[i + 3];
				assert(data.types.find(type) != data.types.end() ||
					data.arrayTypes.find(type) != data.arrayTypes.end() ||
					data.structTypes.find(type) != data.structTypes.end());
				auto foundIntConstant = data.intConstants.find(constantId);
				assert(foundIntConstant != data.intConstants.end());
				SpirArrayInfo& arrayInfo = data.arrayTypes[id];
				arrayInfo.type = type;
				arrayInfo.length = foundIntConstant->second;
				break;
			}
			case spv::OpTypeRuntimeArray:
			{
				assert(wordCount == 3);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t type = spirv[i + 2];
				assert(data.types.find(type) != data.types.end() ||
					data.arrayTypes.find(type) != data.arrayTypes.end() ||
					data.structTypes.find(type) != data.structTypes.end());
				SpirArrayInfo& arrayInfo = data.arrayTypes[id];
				arrayInfo.type = type;
				arrayInfo.length = unknownLength;
				break;
			}
			case spv::OpTypeStruct:
			{
				assert(wordCount >= 2);
				std::uint32_t id = spirv[i + 1];
				std::vector<std::uint32_t>& members = data.structTypes[id];
				members.resize(wordCount - 2);
				for (std::size_t j = 0; j < members.size(); ++j)
				{
					std::uint32_t typeId = spirv[i + 2 + j];
					assert(data.types.find(typeId) != data.types.end() ||
						data.arrayTypes.find(typeId) != data.arrayTypes.end() ||
						data.structTypes.find(typeId) != data.structTypes.end());
					members[j] = typeId;
				}
				break;
			}
			case spv::OpTypeImage:
				readImage(data, spirv, i, wordCount);
				break;
			case spv::OpTypePointer:
			{
				assert(wordCount == 4);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t type = spirv[i + 3];
				switch (spirv[i + 2])
				{
					case spv::StorageClassInput:
					case spv::StorageClassOutput:
					case spv::StorageClassUniform:
					case spv::StorageClassImage:
					case spv::StorageClassPushConstant:
						assert(data.types.find(type) != data.types.end() ||
							data.arrayTypes.find(type) != data.arrayTypes.end() ||
							data.structTypes.find(type) != data.structTypes.end());
						data.pointers[id] = type;
						break;
					default:
						break;
				}
				break;
			}

			// Extract the uniform, input, output, and image variables.
			case spv::OpVariable:
			{
				assert(wordCount >= 4);
				std::uint32_t pointerType = spirv[i + 1];
				std::uint32_t id = spirv[i + 2];
				switch (spirv[i + 3])
				{
					case spv::StorageClassInput:
					{
						auto foundPointer = data.pointers.find(pointerType);
						assert(foundPointer != data.pointers.end());
						data.inputVars[id] = foundPointer->second;
						break;
					}
					case spv::StorageClassOutput:
					{
						auto foundPointer = data.pointers.find(pointerType);
						assert(foundPointer != data.pointers.end());
						data.outputVars[id] = foundPointer->second;
						break;
					}
					case spv::StorageClassUniform:
					{
						auto foundPointer = data.pointers.find(pointerType);
						assert(foundPointer != data.pointers.end());
						data.uniformVars[id] = foundPointer->second;
						break;
					}
					case spv::StorageClassImage:
					{
						auto foundPointer = data.pointers.find(pointerType);
						assert(foundPointer != data.pointers.end());
						data.imageVars[id] = foundPointer->second;
						break;
					}
					case spv::StorageClassPushConstant:
					{
						auto foundPointer = data.pointers.find(pointerType);
						assert(foundPointer != data.pointers.end());
						assert(data.pushConstantPointer.first == unset);
						data.pushConstantPointer = std::make_pair(id, foundPointer->second);
						break;
					}
					default:
						break;
				}
			}

			// Don't care once we reach the function section.
			case spv::OpFunction:
				done = true;
				break;

			default:
				break;
		}

		i += wordCount;
	}

	// Construct our own metadata structures based on what was extracted from SPIR-V.
	addUniforms(*this, data);
}

} // namespace msl
