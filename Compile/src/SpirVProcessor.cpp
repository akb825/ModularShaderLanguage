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
#include <cassert>
#include <unordered_map>
#include <unordered_set>

namespace msl
{

namespace
{

static const unsigned int firstInstruction = 5;
static const unsigned int unknownLength = (unsigned int)-1;
static const std::uint32_t unset = (std::uint32_t)-1;;

struct ArrayInfo
{
	std::uint32_t type;
	unsigned int length;
};

struct IntermediateData
{
	// Names.
	std::unordered_map<std::uint32_t, std::string> names;
	std::unordered_map<std::uint32_t, std::vector<std::string>> memberNames;

	// Type info.
	std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> structTypes;
	std::unordered_map<std::uint32_t, Type> types;
	std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> memberOffsets;
	std::unordered_map<std::uint32_t, std::uint32_t> intConstants;
	std::unordered_map<std::uint32_t, ArrayInfo> arrayTypes;
	std::unordered_set<std::uint32_t> uniformBlocks;
	std::unordered_set<std::uint32_t> uniformBuffers;

	// Metadata.
	std::unordered_map<std::uint32_t, std::uint32_t> bindings;
	std::unordered_map<std::uint32_t, std::uint32_t> locations;

	// Variable declarations.
	std::unordered_map<std::uint32_t, std::uint32_t> pointers;
	std::unordered_map<std::uint32_t, std::uint32_t> uniformVars;
	std::unordered_map<std::uint32_t, std::uint32_t> inputVars;
	std::unordered_map<std::uint32_t, std::uint32_t> outputVars;
	std::unordered_map<std::uint32_t, std::uint32_t> imageVars;
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
	assert(data.types.find(typeId) != data.types.end());
	switch (data.types[typeId])
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
	assert(data.types.find(typeId) != data.types.end());
	switch (data.types[typeId])
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
			assert(data.types.find(typeId) != data.types.end());
			switch (data.types[typeId])
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
			assert(data.types.find(typeId) != data.types.end());
			switch (data.types[typeId])
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
			assert(data.types.find(typeId) != data.types.end());
			switch (data.types[typeId])
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
			assert(data.types.find(typeId) != data.types.end());
			switch (data.types[typeId])
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
			assert(data.types.find(typeId) != data.types.end());
			switch (data.types[typeId])
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
			assert(data.types.find(typeId) != data.types.end());
			switch (data.types[typeId])
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

} // namespace

SpirVProcessor::SpirVProcessor(const std::vector<std::uint32_t>& spirv)
{
	assert(spirv[0] == spv::MagicNumber);
	assert(spirv[1] == spv::Version);
	std::vector<char> tempBuffer;

	IntermediateData data;

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
					case spv::DecorationBinding:
						assert(wordCount == 5);
						data.bindings[id] = spirv[i + 4];
						break;
					case spv::DecorationLocation:
						assert(wordCount == 5);
						data.locations[id] = spirv[i + 4];
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
						assert(wordCount == 5);
						std::vector<std::uint32_t>& thisMemberOffsets = data.memberOffsets[id];
						if (thisMemberOffsets.size() <= member)
							thisMemberOffsets.resize(member + 1, unset);
						thisMemberOffsets[member] = spirv[i + 4];
						break;
				}
				break;
			}

			// Extract integer contsnts.
			case spv::OpConstant:
			{
				assert(wordCount > 3);
				std::uint32_t typeId = spirv[i + 1];
				std::uint32_t id = spirv[i + 2];
				assert(data.types.find(typeId) != data.types.end());
				switch (data.types[typeId])
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
				std::uint32_t typeId = spirv[i + 2];
				std::uint32_t constantId = spirv[i + 3];
				assert(data.types.find(typeId) != data.types.end());
				assert(data.intConstants.find(constantId) != data.intConstants.end());
				ArrayInfo& arrayInfo = data.arrayTypes[id];
				arrayInfo.type = typeId;
				arrayInfo.length = data.intConstants[constantId];
				break;
			}
			case spv::OpTypeRuntimeArray:
			{
				assert(wordCount == 3);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t typeId = spirv[i + 2];
				assert(data.types.find(typeId) != data.types.end());
				ArrayInfo& arrayInfo = data.arrayTypes[id];
				arrayInfo.type = typeId;
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
						assert(data.pointers.find(pointerType) != data.pointers.end());
						data.inputVars[id] = data.pointers[pointerType];
						break;
					case spv::StorageClassOutput:
						assert(data.pointers.find(pointerType) != data.pointers.end());
						data.outputVars[id] = data.pointers[pointerType];
						break;
					case spv::StorageClassUniform:
						assert(data.pointers.find(pointerType) != data.pointers.end());
						data.uniformVars[id] = data.pointers[pointerType];
						break;
					case spv::StorageClassImage:
						assert(data.pointers.find(pointerType) != data.pointers.end());
						data.imageVars[id] = data.pointers[pointerType];
						break;
					case spv::StorageClassPushConstant:
						assert(data.pointers.find(pointerType) != data.pointers.end());
						assert(data.pushConstantPointer.first == unset);
						data.pushConstantPointer = std::make_pair(id, data.pointers[pointerType]);
						break;
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
}

} // namespace msl
