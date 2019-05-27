/*
 * Copyright 2016-2019 Aaron Barany
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
#include <MSL/Compile/Output.h>
#include <SPIRV/spirv.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace msl
{

namespace
{

static const unsigned int minVersion = 0x00010000;
static const unsigned int firstInstruction = 5;
static const unsigned int unknownLength = (unsigned int)-1;

static const char* stageNames[] =
{
	"vertex",
	"tessellation_control",
	"tessellation_evaluation",
	"geometry",
	"fragment",
	"compute"
};
static_assert(sizeof(stageNames)/sizeof(*stageNames) == stageCount,
	"stage name array is out of sync with enum");

static std::uint32_t typeSizes[] =
{
	// Scalars and vectors
	static_cast<std::uint32_t>(sizeof(float)),    // Float
	static_cast<std::uint32_t>(sizeof(float)*2),  // Vec2
	static_cast<std::uint32_t>(sizeof(float)*3),  // Vec3
	static_cast<std::uint32_t>(sizeof(float)*4),  // Vec4
	static_cast<std::uint32_t>(sizeof(double)),   // Double
	static_cast<std::uint32_t>(sizeof(double)*2), // DVec2
	static_cast<std::uint32_t>(sizeof(double)*3), // DVec3
	static_cast<std::uint32_t>(sizeof(double)*4), // DVec4
	static_cast<std::uint32_t>(sizeof(int)),      // Int
	static_cast<std::uint32_t>(sizeof(int)*2),    // IVec2
	static_cast<std::uint32_t>(sizeof(int)*3),    // IVec3
	static_cast<std::uint32_t>(sizeof(int)*4),    // IVec4
	static_cast<std::uint32_t>(sizeof(int)),      // UInt
	static_cast<std::uint32_t>(sizeof(int)*2),    // UVec2
	static_cast<std::uint32_t>(sizeof(int)*3),    // UVec3
	static_cast<std::uint32_t>(sizeof(int)*4),    // UVec4
	static_cast<std::uint32_t>(sizeof(int)),      // Bool
	static_cast<std::uint32_t>(sizeof(int)*2),    // BVec2
	static_cast<std::uint32_t>(sizeof(int)*3),    // BVec3
	static_cast<std::uint32_t>(sizeof(int)*4),    // BVec4

	// Matrices
	static_cast<std::uint32_t>(sizeof(float)*4*2),  // Mat2
	static_cast<std::uint32_t>(sizeof(float)*4*3),  // Mat3
	static_cast<std::uint32_t>(sizeof(float)*4*2),  // Mat4
	static_cast<std::uint32_t>(sizeof(float)*4*3),  // Mat2x3
	static_cast<std::uint32_t>(sizeof(float)*4*4),  // Mat2x4
	static_cast<std::uint32_t>(sizeof(float)*4*2),  // Mat3x2
	static_cast<std::uint32_t>(sizeof(float)*4*4),  // Mat3x4
	static_cast<std::uint32_t>(sizeof(float)*4*2),  // Mat4x2
	static_cast<std::uint32_t>(sizeof(float)*4*3),  // Mat4x3
	static_cast<std::uint32_t>(sizeof(double)*2*2), // DMat2
	static_cast<std::uint32_t>(sizeof(double)*4*2), // DMat3
	static_cast<std::uint32_t>(sizeof(double)*4*4), // DMat4
	static_cast<std::uint32_t>(sizeof(double)*2*3), // DMat2x3
	static_cast<std::uint32_t>(sizeof(double)*2*4), // DMat2x4
	static_cast<std::uint32_t>(sizeof(double)*4*2), // DMat3x2
	static_cast<std::uint32_t>(sizeof(double)*4*4), // DMat3x4
	static_cast<std::uint32_t>(sizeof(double)*4*2), // DMat4x2
	static_cast<std::uint32_t>(sizeof(double)*4*3), // DMat4x3
};

struct SpirArrayInfo
{
	std::uint32_t type;
	unsigned int length;
};

struct MemberInfo
{
	std::uint32_t offset = unknown;
	std::uint32_t matrixStride = unknown;
	bool rowMajor = false;
	bool builtin = false;
	std::uint32_t location = unknown;
	std::uint32_t component = unknown;
};

struct IntermediateData
{
	// Names
	std::unordered_map<std::uint32_t, std::string> names;
	std::unordered_map<std::uint32_t, std::vector<std::string>> memberNames;

	// Type info
	std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> structTypes;
	std::unordered_map<std::uint32_t, Type> types;
	std::unordered_map<std::uint32_t, std::vector<MemberInfo>> members;
	std::unordered_map<std::uint32_t, std::uint32_t> intConstants;
	std::unordered_map<std::uint32_t, SpirArrayInfo> arrayTypes;
	std::unordered_map<std::uint32_t, std::uint32_t> arrayStrides;
	std::unordered_set<std::uint32_t> blocks;
	std::unordered_set<std::uint32_t> uniformBuffers;

	// Metadata
	std::unordered_map<std::uint32_t, std::uint32_t> descriptorSets;
	std::unordered_map<std::uint32_t, std::uint32_t> bindings;
	std::unordered_map<std::uint32_t, std::uint32_t> inputAttachmentIndices;
	std::unordered_map<std::uint32_t, std::uint32_t> locations;
	std::unordered_map<std::uint32_t, std::uint32_t> components;
	std::unordered_set<std::uint32_t> inputOutputStructs;

	// Variable declarations
	// Make these ordered (except for pointers) so they will be consistent across runs.
	std::unordered_map<std::uint32_t, std::uint32_t> pointers;
	std::unordered_set<std::uint32_t> patchVars;
	std::unordered_set<std::uint32_t> builtinVars;
	std::map<std::uint32_t, std::uint32_t> uniformVars;
	std::map<std::uint32_t, std::uint32_t> inputVars;
	std::map<std::uint32_t, std::uint32_t> outputVars;
	std::map<std::uint32_t, std::uint32_t> imageVars;
	std::pair<std::uint32_t, std::uint32_t> pushConstantPointer = std::make_pair(unknown, unknown);
};

bool inputIsArray(Stage stage)
{
	return stage == Stage::TessellationControl || stage == Stage::TessellationEvaluation ||
		stage == Stage::Geometry;
}

bool outputIsArray(Stage stage)
{
	return stage == Stage::TessellationControl;
}

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
	assert(start + wordCount < spirv.size());
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
	MSL_UNUSED(wordCount);
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
	MSL_UNUSED(wordCount);
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
					data.types[id] = Type::Mat3x2;
					break;
				case 4:
					data.types[id] = Type::Mat4x2;
					break;
				default:
					assert(false);
			}
			break;
		case Type::Vec3:
			switch (length)
			{
				case 2:
					data.types[id] = Type::Mat2x3;
					break;
				case 3:
					data.types[id] = Type::Mat3;
					break;
				case 4:
					data.types[id] = Type::Mat4x3;
					break;
				default:
					assert(false);
			}
			break;
		case Type::Vec4:
			switch (length)
			{
				case 2:
					data.types[id] = Type::Mat2x4;
					break;
				case 3:
					data.types[id] = Type::Mat3x4;
					break;
				case 4:
					data.types[id] = Type::Mat4;
					break;
				default:
					assert(false);
			}
			break;
		case Type::DVec2:
			switch (length)
			{
				case 2:
					data.types[id] = Type::DMat2;
					break;
				case 3:
					data.types[id] = Type::DMat3x2;
					break;
				case 4:
					data.types[id] = Type::DMat4x2;
					break;
				default:
					assert(false);
			}
			break;
		case Type::DVec3:
			switch (length)
			{
				case 2:
					data.types[id] = Type::DMat2x3;
					break;
				case 3:
					data.types[id] = Type::DMat3;
					break;
				case 4:
					data.types[id] = Type::DMat4x3;
					break;
				default:
					assert(false);
			}
			break;
		case Type::DVec4:
			switch (length)
			{
				case 2:
					data.types[id] = Type::DMat2x4;
					break;
				case 3:
					data.types[id] = Type::DMat3x4;
					break;
				case 4:
					data.types[id] = Type::DMat4;
					break;
				default:
					assert(false);
			}
			break;
		default:
			assert(false);
			break;
	}
}

void readImage(IntermediateData& data, const std::vector<std::uint32_t>& spirv, std::size_t i,
	std::uint32_t wordCount)
{
	MSL_UNUSED(wordCount);
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
					if (sampled != 2)
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
					if (sampled != 2)
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
					if (sampled != 2)
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
					if (sampled != 2)
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
					if (sampled != 2)
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
					if (sampled != 2)
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
					if (sampled != 2)
						data.types[id] = Type::Sampler3D;
					else
						data.types[id] = Type::Image3D;
					break;
				case Type::Int:
					if (sampled != 2)
						data.types[id] = Type::ISampler3D;
					else
						data.types[id] = Type::IImage3D;
					break;
				case Type::UInt:
					if (sampled != 2)
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
					if (sampled != 2)
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
					if (sampled != 2)
						data.types[id] = Type::ISamplerCube;
					else
						data.types[id] = Type::IImageCube;
					break;
				case Type::UInt:
					if (sampled != 2)
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
					if (sampled != 2)
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
					if (sampled != 2)
						data.types[id] = Type::ISampler2DRect;
					else
						data.types[id] = Type::IImage2DRect;
					break;
				case Type::UInt:
					if (sampled != 2)
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
			if (sampled != 2)
				data.types[id] = Type::SamplerBuffer;
			else
				data.types[id] = Type::ImageBuffer;
			break;
		}
		case spv::DimSubpassData:
		{
			assert(!array);
			assert(sampled == 2);
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
			break;
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

unsigned int getRowCount(Type type)
{
	switch (type)
	{
		case Type::Mat2:
		case Type::Mat3x2:
		case Type::Mat4x2:
		case Type::DMat2:
		case Type::DMat3x2:
		case Type::DMat4x2:
			return 2;
		case Type::Mat3:
		case Type::Mat2x3:
		case Type::Mat4x3:
		case Type::DMat3:
		case Type::DMat2x3:
		case Type::DMat4x3:
			return 3;
		case Type::Mat4:
		case Type::Mat2x4:
		case Type::Mat3x4:
		case Type::DMat4:
		case Type::DMat2x4:
		case Type::DMat3x4:
			return 4;
		default:
			return 0;
	}
}

unsigned int getColumnCount(Type type)
{
	switch (type)
	{
		case Type::Mat2:
		case Type::Mat2x3:
		case Type::Mat2x4:
		case Type::DMat2:
		case Type::DMat2x3:
		case Type::DMat2x4:
			return 2;
		case Type::Mat3:
		case Type::Mat3x2:
		case Type::Mat3x4:
		case Type::DMat3:
		case Type::DMat3x2:
		case Type::DMat3x4:
			return 3;
		case Type::Mat4:
		case Type::Mat4x2:
		case Type::Mat4x3:
		case Type::DMat4:
		case Type::DMat4x2:
		case Type::DMat4x3:
			return 4;
		default:
			return 0;
	}
}

bool isSampledImage(Type type)
{
	switch (type)
	{
		case Type::Sampler1D:
		case Type::Sampler2D:
		case Type::Sampler3D:
		case Type::SamplerCube:
		case Type::Sampler1DShadow:
		case Type::Sampler2DShadow:
		case Type::Sampler1DArray:
		case Type::Sampler2DArray:
		case Type::Sampler1DArrayShadow:
		case Type::Sampler2DArrayShadow:
		case Type::Sampler2DMS:
		case Type::Sampler2DMSArray:
		case Type::SamplerCubeShadow:
		case Type::SamplerBuffer:
		case Type::Sampler2DRect:
		case Type::Sampler2DRectShadow:
		case Type::ISampler1D:
		case Type::ISampler2D:
		case Type::ISampler3D:
		case Type::ISamplerCube:
		case Type::ISampler1DArray:
		case Type::ISampler2DArray:
		case Type::ISampler2DMS:
		case Type::ISampler2DMSArray:
		case Type::ISampler2DRect:
		case Type::USampler1D:
		case Type::USampler2D:
		case Type::USampler3D:
		case Type::USamplerCube:
		case Type::USampler1DArray:
		case Type::USampler2DArray:
		case Type::USampler2DMS:
		case Type::USampler2DMSArray:
		case Type::USampler2DRect:
			return true;
		default:
			return false;
	}
}

bool isImage(Type type)
{
	switch (type)
	{
		case Type::Image1D:
		case Type::Image2D:
		case Type::Image3D:
		case Type::ImageCube:
		case Type::Image1DArray:
		case Type::Image2DArray:
		case Type::Image2DMS:
		case Type::Image2DMSArray:
		case Type::ImageBuffer:
		case Type::Image2DRect:
		case Type::IImage1D:
		case Type::IImage2D:
		case Type::IImage3D:
		case Type::IImageCube:
		case Type::IImage1DArray:
		case Type::IImage2DArray:
		case Type::IImage2DMS:
		case Type::IImage2DMSArray:
		case Type::IImage2DRect:
		case Type::UImage1D:
		case Type::UImage2D:
		case Type::UImage3D:
		case Type::UImageCube:
		case Type::UImage1DArray:
		case Type::UImage2DArray:
		case Type::UImage2DMS:
		case Type::UImage2DMSArray:
		case Type::UImage2DRect:
			return true;
		default:
			return false;
	}
}

bool isSubpassInput(Type type)
{
	switch (type)
	{
		case Type::SubpassInput:
		case Type::SubpassInputMS:
		case Type::ISubpassInput:
		case Type::ISubpassInputMS:
		case Type::USubpassInput:
		case Type::USubpassInputMS:
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
	arrayElements.clear();
	auto foundArray = data.arrayTypes.find(typeId);
	while (foundArray != data.arrayTypes.end())
	{
		arrayElements.emplace_back();
		arrayElements.back().length = foundArray->second.length;

		auto foundArrayStride = data.arrayStrides.find(typeId);
		if (foundArrayStride == data.arrayStrides.end())
			arrayElements.back().stride = unknown;
		else
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

	auto foundMembers = data.members.find(typeId);
	assert(foundMembers == data.members.end() ||
		foundMembers->second.size() <= foundStruct->second.size());

	newStruct.size = 0;
	newStruct.members.resize(foundStruct->second.size());
	for (std::size_t i = 0; i < foundStruct->second.size(); ++i)
	{
		std::uint32_t memberTypeId = foundStruct->second[i];
		StructMember& member = newStruct.members[i];
		member.name = foundMemberNames->second[i];
		if (foundMembers == data.members.end() || i >= foundMembers->second.size())
			member.offset = unknown;
		else
			member.offset = foundMembers->second[i].offset;
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
				if (foundArrayStride == data.arrayStrides.end())
					member.size = unknown;
				else
					member.size = foundArrayStride->second*foundArray->second.length;
			}
		}
		else if (isMatrix(member.type))
		{
			// Matrices have their own strides stored with the struct.
			if (foundMembers != data.members.end() && i < foundMembers->second.size())
			{
				member.rowMajor = foundMembers->second[i].rowMajor;
				if (member.rowMajor)
					member.size = foundMembers->second[i].matrixStride*getRowCount(member.type);
				else
					member.size = foundMembers->second[i].matrixStride*getColumnCount(member.type);
			}
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
		if (newStruct.size != unknown && lastMember.size != unknown)
		{
			newStruct.size += lastMember.size;
			const unsigned int minAlignment = static_cast<unsigned int>(sizeof(float)*4);
			newStruct.size = ((newStruct.size + minAlignment - 1)/minAlignment)*minAlignment;
		}
	}

	assert(processor.structs.size() == processor.structIds.size());
	processor.structs.push_back(std::move(newStruct));
	processor.structIds.push_back(typeId);
	structIndex = static_cast<std::uint32_t>(processor.structs.size() - 1);
	return Type::Struct;
}

std::uint32_t getUnderlyingTypeId(const IntermediateData& data, std::uint32_t typeId)
{
	auto foundArray = data.arrayTypes.find(typeId);
	while (foundArray != data.arrayTypes.end())
	{
		typeId = foundArray->second.type;
		foundArray = data.arrayTypes.find(typeId);
	}

	return typeId;
}

std::vector<std::uint32_t> makeArrayLengths(const std::vector<ArrayInfo>& arrayElements)
{
	std::vector<std::uint32_t> lengths;
	lengths.resize(arrayElements.size());
	for (std::size_t i = 0; i < arrayElements.size(); ++i)
		lengths[i] = arrayElements[i].length;
	return lengths;
}

void addUniforms(SpirVProcessor& processor, const IntermediateData& data)
{
	bool hasPushConstant = data.pushConstantPointer.first != unknown;
	std::size_t totalUniforms = data.uniformVars.size() + data.imageVars.size() + hasPushConstant;
	processor.uniforms.resize(totalUniforms);
	processor.uniformIds.resize(totalUniforms);
	std::size_t i = 0;

	if (hasPushConstant)
	{
		processor.uniformIds[i] = data.pushConstantPointer.first;
		std::uint32_t typeId = data.pushConstantPointer.second;

		Uniform& uniform = processor.uniforms[i];
		uniform.type = getType(uniform.arrayElements, uniform.structIndex, processor, data, typeId);
		assert(uniform.type == Type::Struct);

		uniform.name = processor.structs[uniform.structIndex].name;
		uniform.uniformType = UniformType::PushConstant;
		uniform.descriptorSet = unknown;
		uniform.binding = unknown;
		uniform.inputAttachmentIndex = unknown;
		uniform.samplerIndex = unknown;

		++i;
	}

	for (const std::pair<std::uint32_t, std::uint32_t>& uniformIndices : data.uniformVars)
	{
		processor.uniformIds[i] = uniformIndices.first;
		std::uint32_t typeId = uniformIndices.second;
		std::uint32_t underlyingTypeId = getUnderlyingTypeId(data, typeId);

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

		if (data.blocks.find(underlyingTypeId) != data.blocks.end())
			uniform.uniformType = UniformType::Block;
		else
		{
			assert(data.uniformBuffers.find(underlyingTypeId) != data.uniformBuffers.end());
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

		uniform.inputAttachmentIndex = unknown;
		uniform.samplerIndex = unknown;

		++i;
	}

	for (const std::pair<std::uint32_t, std::uint32_t>& imageIndices : data.imageVars)
	{
		processor.uniformIds[i] = imageIndices.first;
		std::uint32_t typeId = imageIndices.second;

		Uniform& uniform = processor.uniforms[i];

		auto foundName = data.names.find(imageIndices.first);
		assert(foundName != data.names.end());
		uniform.name = foundName->second;

		uniform.type = getType(uniform.arrayElements, uniform.structIndex, processor, data, typeId);

		if (isSampledImage(uniform.type))
			uniform.uniformType = UniformType::SampledImage;
		else if (isSubpassInput(uniform.type))
			uniform.uniformType = UniformType::SubpassInput;
		else
			uniform.uniformType = UniformType::Image;

		auto foundDescriptorSet = data.descriptorSets.find(imageIndices.first);
		if (foundDescriptorSet == data.descriptorSets.end())
			uniform.descriptorSet = unknown;
		else
			uniform.descriptorSet = foundDescriptorSet->second;

		auto foundBinding = data.bindings.find(imageIndices.first);
		if (foundBinding == data.bindings.end())
			uniform.binding = unknown;
		else
			uniform.binding = foundBinding->second;

		auto foundInputAttachmentIndex = data.inputAttachmentIndices.find(imageIndices.first);
		if (foundInputAttachmentIndex == data.inputAttachmentIndices.end())
			uniform.inputAttachmentIndex = unknown;
		else
			uniform.inputAttachmentIndex = foundInputAttachmentIndex->second;

		uniform.samplerIndex = unknown;

		++i;
	}
}

bool addInputsOutputs(Output& output, std::vector<SpirVProcessor::InputOutput>& inputOutputs,
	std::vector<std::uint32_t>& inputOutputIds, SpirVProcessor& processor, IntermediateData& data,
	const std::map<std::uint32_t, std::uint32_t>& inputOutputVars)
{
	std::string ioName = &inputOutputs == &processor.inputs ? "input" : "output";

	inputOutputs.reserve(inputOutputVars.size());
	inputOutputIds.reserve(inputOutputVars.size());
	std::vector<ArrayInfo> arrayElements;
	for (const std::pair<std::uint32_t, std::uint32_t>& inputOutputIndices : inputOutputVars)
	{
		inputOutputs.emplace_back();
		inputOutputIds.emplace_back();
		inputOutputIds.back() = inputOutputIndices.first;
		std::uint32_t typeId = inputOutputIndices.second;
		std::uint32_t underlyingTypeId = getUnderlyingTypeId(data, typeId);

		SpirVProcessor::InputOutput& inputOutput = inputOutputs.back();

		auto foundName = data.names.find(inputOutputIndices.first);
		assert(foundName != data.names.end());
		inputOutput.name = foundName->second;

		inputOutput.type = getType(arrayElements, inputOutput.structIndex, processor, data, typeId);
		inputOutput.arrayElements = makeArrayLengths(arrayElements);
		inputOutput.patch = data.patchVars.find(inputOutputIndices.first) != data.patchVars.end();
		inputOutput.autoAssigned = true;
		if (inputOutput.type == Type::Struct)
		{
			const Struct& structType = processor.structs[inputOutput.structIndex];

			// Make sure any struct is only used once.
			if (data.inputOutputStructs.find(underlyingTypeId) != data.inputOutputStructs.end())
			{
				output.addMessage(Output::Level::Error, processor.fileName, processor.line,
					processor.column, false, "linker error: struct " + structType.name +
					" is used for multiple inputs and outputs");
				return false;
			}
			data.inputOutputStructs.insert(underlyingTypeId);

			// Make sure there's no recursive structs.
			for (const StructMember& member : structType.members)
			{
				if (member.type == Type::Struct)
				{
					output.addMessage(Output::Level::Error, processor.fileName, processor.line,
						processor.column, false,
						"linker error: " + ioName + " member " + structType.name + "." +
						member.name + " is a struct");
					return false;
				}
			}

			// Don't allow arbitrary arrays of input/output blocks.
			bool shouldBeArray = !inputOutput.patch && (&inputOutputs == &processor.inputs ?
				inputIsArray(processor.stage) : outputIsArray(processor.stage));
			if (!inputOutput.arrayElements.empty() != shouldBeArray)
			{
				if (shouldBeArray)
				{
					output.addMessage(Output::Level::Error, processor.fileName, processor.line,
						processor.column, false,
						"linker error: " + ioName + " interface block " + structType.name +
						" must be an array");
				}
				else
				{
					output.addMessage(Output::Level::Error, processor.fileName, processor.line,
						processor.column, false,
						"linker error: " + ioName + " interface block " + structType.name +
						" must not be an array");
				}
				return false;
			}

			inputOutput.memberLocations.resize(structType.members.size(),
				std::make_pair(unknown, 0));
			auto foundMembers = data.members.find(processor.structIds[inputOutput.structIndex]);
			if (foundMembers != data.members.end())
			{
				// Ignore structs with builtin variables.
				if (foundMembers != data.members.end() && foundMembers->second[0].builtin)
				{
					inputOutputs.pop_back();
					inputOutputIds.pop_back();
					continue;
				}

				assert(foundMembers->second.size() <= inputOutput.memberLocations.size());
				for (std::size_t j = 0; j < foundMembers->second.size(); ++j)
				{
					if (foundMembers->second[j].location == unknown)
						continue;

					inputOutput.autoAssigned = false;
					inputOutput.memberLocations[j].first = foundMembers->second[j].location;
					if (foundMembers->second[j].component != unknown)
						inputOutput.memberLocations[j].second = foundMembers->second[j].component;
				}
			}

			auto foundLocation = data.locations.find(inputOutputIndices.first);
			if (foundLocation == data.locations.end())
				inputOutput.location = unknown;
			else
			{
				inputOutput.location = foundLocation->second;
				inputOutput.autoAssigned = false;
			}
			inputOutput.component = unknown;
		}
		else
		{
			// Ignore builtin variables.
			if (data.builtinVars.find(inputOutputIndices.first) != data.builtinVars.end())
			{
				inputOutputs.pop_back();
				inputOutputIds.pop_back();
				continue;
			}

			inputOutput.component = 0;
			auto foundLocation = data.locations.find(inputOutputIndices.first);
			if (foundLocation == data.locations.end())
				inputOutput.location = unknown;
			else
			{
				inputOutput.autoAssigned = false;
				inputOutput.location = foundLocation->second;
				auto foundComponent = data.components.find(inputOutputIndices.first);
				if (foundComponent != data.components.end())
					inputOutput.component = foundComponent->second;
			}
		}
	}

	return true;
}

bool addInputs(Output& output, SpirVProcessor& processor, IntermediateData& data)
{
	return addInputsOutputs(output, processor.inputs, processor.inputIds, processor, data,
		data.inputVars);
}

bool addOutputs(Output& output, SpirVProcessor& processor, IntermediateData& data)
{
	return addInputsOutputs(output, processor.outputs, processor.outputIds, processor, data,
		data.outputVars);
}

void addPushConstants(SpirVProcessor& processor, const IntermediateData& data)
{
	if (data.pushConstantPointer.first == unknown)
	{
		processor.pushConstantStruct = unknown;
		return;
	}

	std::vector<ArrayInfo> arrayElements;
	Type type = getType(arrayElements, processor.pushConstantStruct, processor, data,
		data.pushConstantPointer.second);
	assert(type == Type::Struct);
	MSL_UNUSED(type);
	assert(arrayElements.empty());
}

bool addComponents(std::vector<std::uint8_t>& locations, std::size_t curLocation,
	std::uint32_t componentMask)
{
	if (locations.size() <= curLocation)
		locations.resize(curLocation + 1, 0);

	if (locations[curLocation] & componentMask)
		return false;

	locations[curLocation] |= static_cast<std::uint8_t>(componentMask);
	return true;
}

bool fillLocation(std::vector<std::uint8_t>& locations, std::size_t& curLocation,
	std::uint32_t component, Type type, const std::vector<std::uint32_t>& arrayElements,
	bool removeFirstArray)
{
	assert(component < 4);

	// Get the total number of elements.
	std::uint32_t elementCount = 1;
	for (std::size_t i = removeFirstArray; i < arrayElements.size(); ++i)
	{
		assert(arrayElements[i] != 0);
		if (arrayElements[i] == unknown)
			return false;
		elementCount *= arrayElements[i];
	}

	// Treat matrices the same as arrays.
	switch (type)
	{
		case Type::Mat2:
			type = Type::Vec2;
			elementCount *= 2;
			break;
		case Type::Mat3:
			type = Type::Vec3;
			elementCount *= 3;
			break;
		case Type::Mat4:
			type = Type::Vec4;
			elementCount *= 4;
			break;
		case Type::Mat2x3:
			type = Type::Vec3;
			elementCount *= 2;
			break;
		case Type::Mat2x4:
			type = Type::Vec4;
			elementCount *= 2;
			break;
		case Type::Mat3x2:
			type = Type::Vec2;
			elementCount *= 3;
			break;
		case Type::Mat3x4:
			type = Type::Vec4;
			elementCount *= 3;
			break;
		case Type::Mat4x2:
			type = Type::Vec2;
			elementCount *= 4;
			break;
		case Type::Mat4x3:
			type = Type::Vec3;
			elementCount *= 4;
			break;
		case Type::DMat2:
			type = Type::DVec2;
			elementCount *= 2;
			break;
		case Type::DMat3:
			type = Type::DVec3;
			elementCount *= 3;
			break;
		case Type::DMat4:
			type = Type::DVec4;
			elementCount *= 4;
			break;
		case Type::DMat2x3:
			type = Type::DVec3;
			elementCount *= 2;
			break;
		case Type::DMat2x4:
			type = Type::DVec4;
			elementCount *= 2;
			break;
		case Type::DMat3x2:
			type = Type::DVec2;
			elementCount *= 3;
			break;
		case Type::DMat3x4:
			type = Type::DVec4;
			elementCount *= 3;
			break;
		case Type::DMat4x2:
			type = Type::DVec2;
			elementCount *= 4;
			break;
		case Type::DMat4x3:
			type = Type::DVec3;
			elementCount *= 4;
			break;
		default:
			break;
	}

	switch (type)
	{
		case Type::Float:
		case Type::Int:
		case Type::UInt:
		case Type::Bool:
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation, 1 << component))
					return false;
			}
			break;

		case Type::Vec2:
		case Type::IVec2:
		case Type::UVec2:
		case Type::BVec2:
			if (component > 2)
				return false;
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation, (1 << component) | (2 << component)))
					return false;
			}
			break;

		case Type::Vec3:
		case Type::IVec3:
		case Type::UVec3:
		case Type::BVec3:
			if (component > 1)
				return false;
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation,
					(1 << component) | (2 << component) | (4 << component)))
					return false;
			}
			break;

		case Type::Vec4:
		case Type::IVec4:
		case Type::UVec4:
		case Type::BVec4:
			if (component != 0)
				return false;
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation, 0xF))
					return false;
			}
			break;

		case Type::Double:
			if (component != 0 && component != 2)
				return false;
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation, (1 << component) | (2 << component)))
					return false;
			}
			break;

		case Type::DVec2:
			if (component != 0)
				return false;
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation, 0xF))
					return false;
			}
			break;

		case Type::DVec3:
			if (component != 0)
				return false;
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation, 0xF))
					return false;
				if (!addComponents(locations, ++curLocation, 0x3))
					return false;
			}
			break;

		case Type::DVec4:
			if (component != 0)
				return false;
			for (std::uint32_t i = 0; i < elementCount; ++i, ++curLocation)
			{
				if (!addComponents(locations, curLocation, 0xF))
					return false;
				if (!addComponents(locations, ++curLocation, 0xF))
					return false;
			}
			break;

		default:
			assert(false);
			return false;
	}

	return true;
}

bool assignInputsOutputs(Output& output, const SpirVProcessor& processor,
	std::vector<SpirVProcessor::InputOutput>& inputsOutputs, bool removeFirstArray)
{
	std::string ioName = &inputsOutputs == &processor.inputs ? "input" : "output";
	std::size_t curLocation = 0;
	std::vector<std::uint8_t> locations;
	bool hasExplicitLocations = false;
	bool hasImplicitLocations = false;

	for (SpirVProcessor::InputOutput& io : inputsOutputs)
	{
		if (io.type == Type::Struct)
		{
			const Struct& ioStruct = processor.structs[io.structIndex];
			if (io.memberLocations.empty() || io.memberLocations[0].first == unknown)
			{
				if (io.location == unknown)
					hasImplicitLocations = true;
				else
				{
					curLocation = io.location;
					hasExplicitLocations = true;
				}
			}
			else
			{
				assert(ioStruct.members.size() == io.memberLocations.size());
				hasExplicitLocations = true;
			}

			for (std::size_t i = 0; i < ioStruct.members.size(); ++i)
			{
				std::uint32_t component = 0;
				if (io.memberLocations[i].first == unknown)
				{
					io.memberLocations[i].first = static_cast<std::uint32_t>(curLocation);
					io.memberLocations[i].second = component;
				}
				else
				{
					curLocation = io.memberLocations[i].first;
					component = io.memberLocations[i].second;
				}

				if (!fillLocation(locations, curLocation, component, ioStruct.members[i].type,
					makeArrayLengths(ioStruct.members[i].arrayElements), false))
				{
					output.addMessage(Output::Level::Error, processor.fileName, processor.line,
						processor.column, false,
						"linker error: cannot assign location for " + ioName + " block element " +
						ioStruct.name + "." + ioStruct.members[i].name);
					return false;
				}
			}
		}
		else
		{
			std::uint32_t component = 0;
			if (io.location == unknown)
			{
				io.location = static_cast<std::uint32_t>(curLocation);
				io.component = component;
				hasImplicitLocations = true;
			}
			else
			{
				curLocation = io.location;
				component = io.component;
				hasExplicitLocations = true;
			}

			if (!fillLocation(locations, curLocation, component, io.type, io.arrayElements,
				removeFirstArray))
			{
				output.addMessage(Output::Level::Error, processor.fileName, processor.line,
					processor.column, false,
					"linker error: cannot assign location for " + ioName + " " + io.name);
				return false;
			}
		}
	}

	if (hasImplicitLocations && hasExplicitLocations)
	{
		output.addMessage(Output::Level::Error, processor.fileName, processor.line,
			processor.column, false,
			"linker error: " + ioName + " declarations mix implicit and explicit locations " +
			"in stage " + stageNames[static_cast<unsigned int>(processor.stage)]);
		return false;
	}

	return true;
}

bool findLinkedMember(Output& output, std::uint32_t& outputIndex, std::uint32_t& memberIndex,
	const SpirVProcessor& processor, const std::string& name)
{
	outputIndex = unknown;
	memberIndex = unknown;

	for (std::uint32_t i = 0; i < processor.outputs.size(); ++i)
	{
		if (processor.outputs[i] .type != Type::Struct)
			continue;

		const Struct& outputStruct = processor.structs[processor.outputs[i].structIndex];
		for (std::uint32_t j = 0; j < outputStruct.members.size(); ++j)
		{
			if (outputStruct.members[j].name == name)
			{
				if (outputIndex != unknown)
				{
					output.addMessage(Output::Level::Error, processor.fileName, processor.line,
						processor.column, false,
						"linker error: multiple members from output interface blocks match the "
						"name " + name + " in stage " +
						stageNames[static_cast<unsigned int>(processor.stage)]);
					return false;
				}

				outputIndex = i;
				memberIndex = j;
			}
		}
	}

	if (outputIndex == unknown)
	{
		output.addMessage(Output::Level::Error, processor.fileName, processor.line,
			processor.column, false,
			"linker error: cannot find output interface block member with name " + name +
			" in stage " + stageNames[static_cast<unsigned int>(processor.stage)]);
		return false;
	}

	assert(memberIndex != unknown);
	return true;
}

bool inputOutputArraysEqual(const std::vector<std::uint32_t>& outputArray, bool removeFirstOutput,
	const std::vector<std::uint32_t>& inputArray, bool removeFirstInput)
{
	if (removeFirstOutput && outputArray.empty())
		return false;
	if (removeFirstInput && inputArray.empty())
		return false;

	if (outputArray.size() - removeFirstOutput != inputArray.size() - removeFirstInput)
		return false;

	for (std::size_t i = removeFirstOutput; i < outputArray.size(); ++i)
	{
		if (outputArray[i] != inputArray[i - removeFirstOutput + removeFirstInput])
			return false;
	}

	return true;
}

void addDummyDescriptorSet(std::vector<std::uint32_t>& spirv, std::uint32_t id)
{
	spirv.push_back((4 << spv::WordCountShift) | spv::OpDecorate);
	spirv.push_back(id);
	spirv.push_back(spv::DecorationDescriptorSet);
	spirv.push_back(unknown);
}

void addDummyBinding(std::vector<std::uint32_t>& spirv, std::uint32_t id)
{
	spirv.push_back((4 << spv::WordCountShift) | spv::OpDecorate);
	spirv.push_back(id);
	spirv.push_back(spv::DecorationBinding);
	spirv.push_back(unknown);
}

void addLocation(std::vector<std::uint32_t>& spirv, std::uint32_t id, std::uint32_t index)
{
	spirv.push_back((4 << spv::WordCountShift) | spv::OpDecorate);
	spirv.push_back(id);
	spirv.push_back(spv::DecorationLocation);
	spirv.push_back(index);
}

void addComponent(std::vector<std::uint32_t>& spirv, std::uint32_t id, std::uint32_t index)
{
	if (index == 0)
		return;

	spirv.push_back((4 << spv::WordCountShift) | spv::OpDecorate);
	spirv.push_back(id);
	spirv.push_back(spv::DecorationComponent);
	spirv.push_back(index);
}

void addMemberLocation(std::vector<std::uint32_t>& spirv, std::uint32_t id,
	std::uint32_t memberIndex, std::uint32_t index)
{
	spirv.push_back((5 << spv::WordCountShift) | spv::OpMemberDecorate);
	spirv.push_back(id);
	spirv.push_back(memberIndex);
	spirv.push_back(spv::DecorationLocation);
	spirv.push_back(index);
}

void addMemberComponent(std::vector<std::uint32_t>& spirv, std::uint32_t id,
	std::uint32_t memberIndex, std::uint32_t index)
{
	if (index == 0)
		return;

	spirv.push_back((5 << spv::WordCountShift) | spv::OpMemberDecorate);
	spirv.push_back(id);
	spirv.push_back(memberIndex);
	spirv.push_back(spv::DecorationComponent);
	spirv.push_back(index);
}

} // namespace

namespace compile
{

inline bool operator==(const ArrayInfo& info1, const ArrayInfo& info2)
{
	return info1.length == info2.length && info1.stride == info2.stride;
}

inline bool operator!=(const ArrayInfo& info1, const ArrayInfo& info2)
{
	return !(info1 == info2);
}

} // namespace compile

bool SpirVProcessor::extract(Output& output, const std::string& fileName, std::size_t line,
	std::size_t column, const std::vector<std::uint32_t>& spirv, Stage stage)
{
	this->stage = stage;
	this->fileName = fileName;
	this->line = line;
	this->column = column;
	this->spirv = &spirv;

	MSL_UNUSED(minVersion);
	assert(spirv.size() > firstInstruction);
	assert(spirv[0] == spv::MagicNumber);
	assert(spirv[1] >= minVersion && spirv[1] <= spv::Version);
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
						assert(wordCount == 4);
						data.descriptorSets[id] = spirv[i + 3];
						break;
					case spv::DecorationBinding:
						assert(wordCount == 4);
						data.bindings[id] = spirv[i + 3];
						break;
					case spv::DecorationInputAttachmentIndex:
						assert(wordCount == 4);
						data.inputAttachmentIndices[id] = spirv[i + 3];
						break;
					case spv::DecorationLocation:
						assert(wordCount == 4);
						data.locations[id] = spirv[i + 3];
						break;
					case spv::DecorationComponent:
						assert(wordCount == 4);
						data.components[id] = spirv[i + 3];
						break;
					case spv::DecorationArrayStride:
						assert(wordCount == 4);
						data.arrayStrides[id] = spirv[i + 3];
						break;
					case spv::DecorationBlock:
						data.blocks.insert(id);
						break;
					case spv::DecorationBufferBlock:
						data.uniformBuffers.insert(id);
						break;
					case spv::DecorationPatch:
						data.patchVars.insert(id);
						break;
					case spv::DecorationBuiltIn:
						data.builtinVars.insert(id);
						break;
				}
				break;
			}
			case spv::OpMemberDecorate:
			{
				assert(wordCount >= 4);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t member = spirv[i + 2];
				std::vector<MemberInfo>& memberInfo = data.members[id];
				if (memberInfo.size() <= member)
					memberInfo.resize(member + 1);
				switch (spirv[i + 3])
				{
					case spv::DecorationOffset:
					{
						assert(wordCount == 5);
						memberInfo[member].offset = spirv[i + 4];
						break;
					}
					case spv::DecorationMatrixStride:
					{
						assert(wordCount == 5);
						memberInfo[member].matrixStride = spirv[i + 4];
						break;
					}
					case spv::DecorationLocation:
					{
						assert(wordCount == 5);
						memberInfo[member].location = spirv[i + 4];
						break;
					}
					case spv::DecorationComponent:
					{
						assert(wordCount == 5);
						memberInfo[member].component = spirv[i + 4];
						break;
					}
					case spv::DecorationRowMajor:
					{
						memberInfo[member].rowMajor = true;
						break;
					}
					case spv::DecorationColMajor:
					{
						memberInfo[member].rowMajor = false;
						break;
					}
					case spv::DecorationBuiltIn:
					{
						memberInfo[member].builtin = true;
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
			case spv::OpTypeSampledImage:
			{
				assert(wordCount >= 3);
				std::uint32_t id = spirv[i + 1];
				std::uint32_t typeId = spirv[i + 2];
				auto foundType = data.types.find(typeId);
				assert(foundType != data.types.end());
				data.types[id] = foundType->second;
				break;
			}
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
					case spv::StorageClassUniformConstant:
					case spv::StorageClassPushConstant:
						if (data.types.find(type) != data.types.end() ||
							data.arrayTypes.find(type) != data.arrayTypes.end() ||
							data.structTypes.find(type) != data.structTypes.end())
						{
							data.pointers[id] = type;
						}
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
					case spv::StorageClassUniformConstant:
					{
						auto foundPointer = data.pointers.find(pointerType);
						assert(foundPointer != data.pointers.end());
						std::vector<ArrayInfo> arrayElements;
						std::uint32_t structIndex;
						Type type = getType(arrayElements, structIndex, *this, data,
							foundPointer->second);
						if (isImage(type) || isSampledImage(type) || isSubpassInput(type))
							data.imageVars[id] = foundPointer->second;
						break;
					}
					case spv::StorageClassPushConstant:
					{
						auto foundPointer = data.pointers.find(pointerType);
						assert(foundPointer != data.pointers.end());
						assert(data.pushConstantPointer.first == unknown);
						data.pushConstantPointer = std::make_pair(id, foundPointer->second);
						break;
					}
					default:
						break;
				}
				break;
			}

			// Extract compute local size.
			case spv::OpExecutionMode:
				assert(wordCount >= 3);
				if (spirv[i + 2] == spv::ExecutionModeLocalSize)
				{
					assert(wordCount == 6);
					computeLocalSize[0] = spirv[i + 3];
					computeLocalSize[1] = spirv[i + 4];
					computeLocalSize[2] = spirv[i + 5];
				}
				break;

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
	if (!addInputs(output, *this, data))
		return false;
	if (!addOutputs(output, *this, data))
		return false;
	addPushConstants(*this, data);

	// Sanity checks:
	const char* builtinPrefix = "gl_";
	std::size_t builtinPrefixLen = std::strlen(builtinPrefix);
	std::unordered_set<std::string> encounteredNames;
	for (const Struct& thisStruct : structs)
	{
		if (thisStruct.name.compare(0, builtinPrefixLen, builtinPrefix) == 0)
			continue;

		if (!encounteredNames.insert(thisStruct.name).second)
		{
			output.addMessage(Output::Level::Error, fileName, line, column, false,
				"linker error: multiple sructs of name " + thisStruct.name + " declared; this "
				"could be due to using the same struct in different contexts, such a uniform "
				"block and uniform buffer");
			return false;
		}
	}

	encounteredNames.clear();
	for (const Uniform& uniform : uniforms)
	{
		if (!encounteredNames.insert(uniform.name).second)
		{
			output.addMessage(Output::Level::Error, fileName, line, column, false,
				"linker error: multiple uniforms of name " + uniform.name + " declared");
			return false;
		}
	}

	encounteredNames.clear();
	for (const InputOutput& stageInput : inputs)
	{
		if (stageInput.type == Type::Struct)
			continue;

		if (!encounteredNames.insert(stageInput.name).second)
		{
			output.addMessage(Output::Level::Error, fileName, line, column, false,
				"linker error: multiple inputs of name " + stageInput.name + "in stage " +
				stageNames[static_cast<unsigned int>(stage)]);
			return false;
		}
	}

	encounteredNames.clear();
	for (const InputOutput& stageOutput : outputs)
	{
		if (stageOutput.type == Type::Struct)
			continue;

		if (!encounteredNames.insert(stageOutput.name).second)
		{
			output.addMessage(Output::Level::Error, fileName, line, column, false,
				"linker error: multiple outputs of name " + stageOutput.name + "in stage " +
				stageNames[static_cast<unsigned int>(stage)]);
			return false;
		}
	}

	return true;
}

bool SpirVProcessor::uniformsCompatible(Output& output, const SpirVProcessor& other) const
{
	bool success = true;

	// Uniforms
	for (const Uniform& uniform : uniforms)
	{
		for (const Uniform& otherUniform : other.uniforms)
		{
			if (uniform.name != otherUniform.name)
				continue;

			if (uniform.uniformType != otherUniform.uniformType ||
				uniform.type != otherUniform.type ||
				uniform.arrayElements != otherUniform.arrayElements ||
				uniform.descriptorSet != otherUniform.descriptorSet ||
				uniform.binding != otherUniform.binding ||
				(uniform.type == Type::Struct && structs[uniform.structIndex].name !=
					other.structs[otherUniform.structIndex].name))
			{
				output.addMessage(Output::Level::Error, fileName, line, column, false,
					"linker error: uniform " + uniform.name +
					" has different declarations between stages");
				success = false;
			}

			break;
		}
	}

	// Structs
	for (const Struct& thisStruct : structs)
	{
		for (const Struct& otherStruct : other.structs)
		{
			if (thisStruct.name != otherStruct.name)
				continue;

			bool compatible = true;
			if (thisStruct.size != otherStruct.size ||
				thisStruct.members.size() != otherStruct.members.size())
			{
				compatible = false;
			}

			if (compatible)
			{
				for (std::size_t i = 0; i < thisStruct.members.size(); ++i)
				{
					const StructMember& thisMember = thisStruct.members[i];
					const StructMember& otherMember = otherStruct.members[i];
					if (thisMember.name != otherMember.name ||
						thisMember.offset != otherMember.offset ||
						thisMember.size != otherMember.size ||
						thisMember.type != otherMember.type ||
						thisMember.arrayElements != otherMember.arrayElements ||
						(thisMember.type == Type::Struct && structs[thisMember.structIndex].name !=
							other.structs[otherMember.structIndex].name))
					{
						compatible = false;
						break;
					}
				}
			}

			if (!compatible)
			{
				output.addMessage(Output::Level::Error, fileName, line, column, false,
					"linker error: struct " + thisStruct.name +
					" has different declarations between stages");
				success = false;
			}

			break;
		}
	}

	return success;
}

bool SpirVProcessor::assignInputs(Output& output)
{
	return assignInputsOutputs(output, *this, inputs, inputIsArray(stage));
}

bool SpirVProcessor::assignOutputs(Output& output)
{
	return assignInputsOutputs(output, *this, outputs, outputIsArray(stage));
}

bool SpirVProcessor::linkInputs(Output& output, const SpirVProcessor& prevStage)
{
	bool success = true;
	bool inputArrays = inputIsArray(stage);
	bool outputArrays = outputIsArray(prevStage.stage);
	for (InputOutput& input : inputs)
	{
		if (input.type == Type::Struct)
		{
			Struct& inputStruct = structs[input.structIndex];
			assert(inputStruct.members.size() == input.memberLocations.size());
			for (std::size_t i = 0; i < inputStruct.members.size(); ++i)
			{
				if (input.memberLocations[i].first != unknown)
					continue;

				std::uint32_t otherOutIndex, otherMemberIndex;
				if (!findLinkedMember(output, otherOutIndex, otherMemberIndex, prevStage,
					inputStruct.members[i].name))
				{
					success = false;
					continue;
				}

				const Struct& outputStruct =
					prevStage.structs[prevStage.outputs[otherOutIndex].structIndex];
				if (inputStruct.members[i].type != outputStruct.members[otherMemberIndex].type ||
					input.patch != prevStage.outputs[otherOutIndex].patch ||
					!inputOutputArraysEqual(
						makeArrayLengths(outputStruct.members[otherMemberIndex].arrayElements),
						false, makeArrayLengths(inputStruct.members[i].arrayElements), false))
				{
					output.addMessage(Output::Level::Error, fileName, line, column, false,
						"linker error: type mismatch when linking input member " +
						inputStruct.name + "." + inputStruct.members[i].name + " in stage " +
						stageNames[static_cast<unsigned int>(stage)]);
					success = false;
					continue;
				}

				input.memberLocations[i] =
					prevStage.outputs[otherOutIndex].memberLocations[otherMemberIndex];
			}
		}
		else
		{
			if (input.location != unknown)
				continue;

			bool found = false;
			for (const InputOutput& out : prevStage.outputs)
			{
				if (input.name != out.name)
					continue;

				found = true;
				if (input.type != out.type || input.patch != out.patch ||
					!inputOutputArraysEqual(out.arrayElements, outputArrays && !out.patch,
					input.arrayElements, inputArrays && !input.patch))
				{
					output.addMessage(Output::Level::Error, fileName, line, column, false,
						"linker error: type mismatch when linking input " + input.name +
						" in stage " + stageNames[static_cast<unsigned int>(stage)]);
					success = false;
					break;
				}

				input.location = out.location;
				input.component = out.component;
				break;
			}

			if (!found)
			{
				output.addMessage(Output::Level::Error, fileName, line, column, false,
					"linker error: cannot find output with name " + input.name + " in stage " +
					stageNames[static_cast<unsigned int>(prevStage.stage)]);
				success = false;
			}
		}
	}

	return success;
}

std::vector<std::uint32_t> SpirVProcessor::process(Strip strip, bool dummyBindings) const
{
	std::unordered_set<std::uint32_t> keepNames;
	if (strip == Strip::AllButReflection)
	{
		for (std::uint32_t id : structIds)
			keepNames.insert(id);
		for (std::uint32_t id : uniformIds)
			keepNames.insert(id);
		for (std::uint32_t id : inputIds)
			keepNames.insert(id);
		for (std::uint32_t id : outputIds)
			keepNames.insert(id);
	}

	std::vector<std::uint32_t> result;
	result.insert(result.end(), spirv->begin(), spirv->begin() + firstInstruction);

	std::unordered_map<std::uint32_t, std::uint32_t> locations;
	std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> memberLocations;

	unsigned int wordCount;
	bool endOfAnnotations = false;
	for (std::size_t i = firstInstruction; i < spirv->size(); i += wordCount)
	{
		spv::Op op = getOp((*spirv)[i]);
		wordCount = getWordCount((*spirv)[i]);
		assert(wordCount > 0 && wordCount + i <= spirv->size());
		if (endOfAnnotations)
		{
			result.insert(result.end(), spirv->begin() + i, spirv->begin() + i + wordCount);
			continue;
		}

		switch (op)
		{
			// Strip debug info.
			case spv::OpSource:
			case spv::OpSourceContinued:
			case spv::OpSourceExtension:
			case spv::OpString:
			case spv::OpLine:
				if (strip == Strip::None)
					result.insert(result.end(), spirv->begin() + i, spirv->begin() + i + wordCount);
				break;

			// Strip names.
			case spv::OpName:
			case spv::OpMemberName:
			{
				assert(wordCount >= 3);
				std::uint32_t id = (*spirv)[i + 1];
				if (strip == Strip::None || (strip == Strip::AllButReflection &&
					keepNames.find(id) != keepNames.end()))
				{
					result.insert(result.end(), spirv->begin() + i, spirv->begin() + i + wordCount);
				}
				break;
			}

			// Keep track of existing input/output locations.
			case spv::OpDecorate:
			{
				assert(wordCount >= 3);
				std::uint32_t id = (*spirv)[i + 1];
				switch ((*spirv)[i + 2])
				{
					case spv::DecorationLocation:
						assert(wordCount == 4);
						locations[id] = (*spirv)[i + 3];
						break;
				}
				result.insert(result.end(), spirv->begin() + i, spirv->begin() + i + wordCount);
				break;
			}
			case spv::OpMemberDecorate:
			{
				assert(wordCount >= 4);
				std::uint32_t id = (*spirv)[i + 1];
				std::uint32_t member = (*spirv)[i + 2];
				switch ((*spirv)[i + 3])
				{
					case spv::DecorationLocation:
					{
						assert(wordCount == 5);
						std::vector<std::uint32_t>& memberLocation = memberLocations[id];
						if (memberLocation.size() <= member)
							memberLocation.resize(member + 1);
						memberLocation[member] = (*spirv)[i + 4];
						break;
					}
				}
				result.insert(result.end(), spirv->begin() + i, spirv->begin() + i + wordCount);
				break;
			}

			// Capture other instructions before the end of annotations.
			case spv::OpCapability:
			case spv::OpExtension:
			case spv::OpExtInstImport:
			case spv::OpMemoryModel:
			case spv::OpEntryPoint:
			case spv::OpExecutionMode:
			case spv::OpGroupDecorate:
			case spv::OpGroupMemberDecorate:
			case spv::OpDecorationGroup:
				result.insert(result.end(), spirv->begin() + i, spirv->begin() + i + wordCount);
				break;

			// Finish with the other annotations. Add our own and continue with the rest of
			// SPIR-V.
			default:
				// Add input locations.
				for (std::size_t j = 0; j < inputs.size(); ++j)
				{
					if (!inputs[j].autoAssigned)
						continue;

					if (inputs[j].type == Type::Struct)
					{
						std::uint32_t typeId = structIds[inputs[j].structIndex];
						auto foundMember = memberLocations.find(typeId);
						for (std::uint32_t k = 0; k < inputs[j].memberLocations.size(); ++k)
						{
							if (foundMember == memberLocations.end() ||
								k >= foundMember->second.size() ||
								foundMember->second[k] == unknown)
							{
								addMemberLocation(result, typeId, k,
									inputs[j].memberLocations[k].first);
								addMemberComponent(result, typeId, k,
									inputs[j].memberLocations[k].second);
							}
						}
					}
					else
					{
						if (locations.find(inputIds[j]) == locations.end())
						{
							addLocation(result, inputIds[j], inputs[j].location);
							addComponent(result, inputIds[j], inputs[j].component);
						}
					}
				}

				// Add output locations.
				for (std::size_t j = 0; j < outputs.size(); ++j)
				{
					if (!outputs[j].autoAssigned)
						continue;

					if (outputs[j].type == Type::Struct)
					{
						std::uint32_t typeId = structIds[outputs[j].structIndex];
						auto foundMember = memberLocations.find(typeId);
						for (std::uint32_t k = 0; k < outputs[j].memberLocations.size(); ++k)
						{
							if (foundMember == memberLocations.end() ||
								k >= foundMember->second.size() ||
								foundMember->second[k] == unknown)
							{
								addMemberLocation(result, typeId, k,
									outputs[j].memberLocations[k].first);
								addMemberComponent(result, typeId, k,
									outputs[j].memberLocations[k].second);
							}
						}
					}
					else
					{
						if (locations.find(outputIds[j]) == locations.end())
						{
							addLocation(result, outputIds[j], outputs[j].location);
							addComponent(result, outputIds[j], outputs[j].component);
						}
					}
				}

				// Add dummy bindings.
				if (dummyBindings)
				{
					for (std::size_t j = 0; j < uniforms.size(); ++j)
					{
						if (uniforms[j].structIndex != unknown &&
							uniforms[j].structIndex == pushConstantStruct)
						{
							continue;
						}

						if (uniforms[j].descriptorSet == unknown)
							addDummyDescriptorSet(result, uniformIds[j]);
						if (uniforms[j].binding == unknown)
							addDummyBinding(result, uniformIds[j]);
					}
				}

				endOfAnnotations = true;
				result.insert(result.end(), spirv->begin() + i, spirv->begin() + i + wordCount);
				break;
		}
	}

	return result;
}

} // namespace msl
