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

#include <MSL/Client/ModuleCpp.h>
#include <fstream>
#include <iostream>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#if MSL_CLANG
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
#endif

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic pop
#endif

using namespace boost::program_options;
using namespace boost::filesystem;

static const char* typeNames[] =
{
	// Scalars and vectors
	"float",
	"vec2",
	"vec3",
	"vec4",
	"double",
	"dvec2",
	"dvec3",
	"dvec4",
	"int",
	"ivec2",
	"ivec3",
	"ivec4",
	"unsigned int",
	"uvec2",
	"uvec3",
	"uvec4",
	"bool",
	"bvec2",
	"bvec3",
	"bvec4",

	// Matrices
	"mat2",
	"mat3",
	"mat4",
	"mat2x3",
	"mat2x4",
	"mat3x2",
	"mat3x4",
	"mat4x2",
	"mat4x3",
	"dmat2",
	"dmat3",
	"dmat4",
	"dmat2x3",
	"dmat2x4",
	"dmat3x2",
	"dmat3x4",
	"dmat4x2",
	"dmat4x3",

	// Samplers
	"sampler1D",
	"sampler2D",
	"sampler3D",
	"samplerCube",
	"sampler1DShadow",
	"sampler2DShadow",
	"sampler1DArray",
	"sampler2DArray",
	"sampler1DArrayShadow",
	"sampler2DArrayShadow",
	"sampler2DMS",
	"sampler2DMSArray",
	"samplerCubeShadow",
	"samplerBuffer",
	"sampler2DRect",
	"sampler2DRectShadow",
	"isampler1D",
	"isampler2D",
	"isampler3D",
	"isamplerCube",
	"isampler1DArray",
	"isampler2DArray",
	"isampler2DMS",
	"isampler2DMSArray",
	"isampler2DRect",
	"usampler1D",
	"usampler2D",
	"usampler3D",
	"usamplerCube",
	"usampler1DArray",
	"usampler2DArray",
	"usampler2DMS",
	"usampler2DMSArray",
	"usampler2DRect",

	// Images
	"image1D",
	"image2D",
	"image3D",
	"imageCube",
	"image1DArray",
	"image2DArray",
	"image2DMS",
	"image2DMSArray",
	"imageBuffer",
	"image2DRect",
	"iimage1D",
	"iimage2D",
	"iimage3D",
	"iimageCube",
	"iimage1DArray",
	"iimage2DArray",
	"iimage2DMS",
	"iimage2DMSArray",
	"iimage2DRect",
	"uimage1D",
	"uimage2D",
	"uimage3D",
	"uimageCube",
	"uimage1DArray",
	"uimage2DArray",
	"uimage2DMS",
	"uimage2DMSArray",
	"uimage2DRect",

	// Subpass inputs
	"subpassInput",
	"subpassInputMS",
	"isubpassInput",
	"isubpassInputMS",
	"usubpassInput",
	"usubpassInputMS",

	// Other
	"struct",
};
static_assert(sizeof(typeNames)/sizeof(*typeNames) == msl::typeCount,
	"typeNames out of sync with enum");

static const char* uniformTypeNames[] =
{
	"push_constant",
	"block",
	"block_buffer",
	"image",
	"sampled_image",
	"subpass_image"
};
static_assert(sizeof(uniformTypeNames)/sizeof(*uniformTypeNames) ==
	static_cast<int>(msl::UniformType::SubpassInput) + 1, "uniformTypeNames out of sync with enum");

static const char* boolNames[] =
{
	"false",
	"true"
};
static_assert(sizeof(boolNames)/sizeof(*boolNames) == static_cast<int>(msl::Bool::True) + 1,
	"boolNames out of sync with enum");

static const char* polygonModeNames[] =
{
	"fill",
	"line",
	"point"
};
static_assert(sizeof(polygonModeNames)/sizeof(*polygonModeNames) ==
	static_cast<int>(msl::PolygonMode::Point) + 1, "polygonModeNames out of sync with enum");

static const char* cullModeNames[] =
{
	"none",
	"front",
	"back",
	"front_and_back"
};
static_assert(sizeof(cullModeNames)/sizeof(*cullModeNames) ==
	static_cast<int>(msl::CullMode::FrontAndBack) + 1, "cullModeNames out of sync with enum");

static const char* frontFaceNames[] =
{
	"counter_clockwise",
	"clockwise"
};
static_assert(sizeof(frontFaceNames)/sizeof(*frontFaceNames) ==
	static_cast<int>(msl::FrontFace::Clockwise) + 1, "frontFaceNames out of sync with enum");

static const char* stencilOpNames[] =
{
	"keep",
	"zero",
	"replace",
	"increment_and_clamp",
	"decrement_and_clamp",
	"invert",
	"increment_and_wrap",
	"decrement_and_wrap"
};
static_assert(sizeof(stencilOpNames)/sizeof(*stencilOpNames) ==
	static_cast<int>(msl::StencilOp::DecrementAndWrap) + 1, "stencilOpNames out of sync with enum");

static const char* compareOpNames[] =
{
	"never",
	"less",
	"equal",
	"less_or_equal",
	"greater",
	"not_equal",
	"greater_or_equal",
	"always"
};
static_assert(sizeof(compareOpNames)/sizeof(*compareOpNames) ==
	static_cast<int>(msl::CompareOp::Always) + 1, "compareOpNames out of sync with enum");

static const char* blendFactorNames[] =
{
	"zero",
	"one",
	"src_color",
	"one_minus_src_color",
	"dst_color",
	"one_minus_dst_color",
	"src_alpha",
	"one_minus_src_alpha",
	"dst_alpha",
	"one_minus_dst_alpha",
	"const_color",
	"one_minus_const_color",
	"const_alpha",
	"one_minus_const_alpha",
	"src_alpha_saturate",
	"src1_color",
	"one_minus_src1_color",
	"src1_alpha",
	"one_minus_src1_alpha"
};
static_assert(sizeof(blendFactorNames)/sizeof(*blendFactorNames) ==
	static_cast<int>(msl::BlendFactor::OneMinusSrc1Alpha) + 1,
	"blendFactorNames out of sync with enum");

static const char* blendOpNames[] =
{
	"add",
	"subtract",
	"reverse_subtract",
	"min",
	"max"
};
static_assert(sizeof(blendOpNames)/sizeof(*blendOpNames) ==
	static_cast<int>(msl::BlendOp::Max) + 1, "blendOpNames out of sync with enum");

static const char* logicOpNames[] =
{
	"clear",
	"and",
	"and_reverse",
	"copy",
	"and_inverted",
	"no_op",
	"xor",
	"or",
	"nor",
	"equivalent",
	"invert",
	"or_reverse",
	"copy_inverted",
	"or_inverted",
	"nand",
	"set"
};
static_assert(sizeof(logicOpNames)/sizeof(*logicOpNames) ==
	static_cast<int>(msl::LogicOp::Set) + 1, "logicOpNames out of sync with enum");

static const char* filterNames[] =
{
	"nearest",
	"linear"
};
static_assert(sizeof(filterNames)/sizeof(*filterNames) ==
	static_cast<int>(msl::Filter::Linear) + 1, "filterNames out of sync with enum");

static const char* mipFilterNames[] =
{
	"none",
	"nearest",
	"linear",
	"anisotropic"
};
static_assert(sizeof(mipFilterNames)/sizeof(*mipFilterNames) ==
	static_cast<int>(msl::MipFilter::Anisotropic) + 1, "mipFilterNames out of sync with enum");

static const char* addressModeNames[] =
{
	"repeat",
	"mirrored_repeat",
	"clamp_to_edge",
	"clamp_to_border",
	"mirror_once"
};
static_assert(sizeof(addressModeNames)/sizeof(*addressModeNames) ==
	static_cast<int>(msl::AddressMode::MirrorOnce) + 1, "addressModeNames out of sync with enum");

static const char* borderColorNames[] =
{
	"transparent_black",
	"transparent_int_zero",
	"opaque_black",
	"opaque_int_zero",
	"opaque_white",
	"opaque_int_one"
};
static_assert(sizeof(borderColorNames)/sizeof(*borderColorNames) ==
	static_cast<int>(msl::BorderColor::OpaqueIntOne) + 1, "borderColorNames out of sync with enum");

static const char* stageExtensions[] =
{
	".vert",
	".tesc",
	".tese",
	".geom",
	".frag",
	".comp"
};
static_assert(sizeof(stageExtensions)/sizeof(*stageExtensions) == msl::stageCount,
	"stageExtensions out of sync with enum");

static const char* stageNames[] =
{
	"vertex",
	"tessellation-control",
	"tessellation-evaluation",
	"geometry",
	"fragment",
	"compute"
};
static_assert(sizeof(stageNames)/sizeof(*stageNames) == msl::stageCount,
	"stageNames out of sync with enum");

static bool shadersAreText(const msl::Module& module)
{
	uint32_t targetId = module.targetId();
	return
		targetId == MSL_CREATE_ID('G', 'L', 'S', 'L') ||
		targetId == MSL_CREATE_ID('G', 'L', 'E', 'S');
}

static bool openFile(std::ofstream& stream, const std::string& fileName, bool binary)
{
	std::ios_base::openmode options = std::ios_base::trunc;
	if (binary)
		options |= std::ios_base::binary;
	stream.open(fileName, options);
	if (!stream.is_open())
	{
		std::cerr << "error: could not open file: " << fileName <<std::endl;
		return false;
	}

	return true;
}

static void writeStructs(std::ostream& jsonFile, const msl::Module& module,
	const msl::Pipeline& pipeline, uint32_t i)
{
	jsonFile << "\t\t\t\"structs\":\n\t\t\t[\n";
	for (uint32_t j = 0; j < pipeline.structCount; ++j)
	{
		jsonFile << "\t\t\t\t{\n";
		msl::Struct pipelineStruct;
		module.pipelineStruct(pipelineStruct, i, j);

		jsonFile << "\t\t\t\t\t\"name\": \"" << pipelineStruct.name << "\",\n";
		if (pipelineStruct.size == msl::unknown)
			jsonFile << "\t\t\t\t\t\"size\": null,\n";
		else
			jsonFile << "\t\t\t\t\t\"size\": " << pipelineStruct.size << ",\n";

		jsonFile << "\t\t\t\t\t\"members\":\n\t\t\t\t\t[\n";

		for (uint32_t k = 0; k < pipelineStruct.memberCount; ++k)
		{
			jsonFile << "\t\t\t\t\t\t{\n";
			msl::StructMember structMember;
			module.structMember(structMember, i, j, k);
			jsonFile << "\t\t\t\t\t\t\t\"name\": \"" << structMember.name << "\",\n";

			if (structMember.offset == msl::unknown)
				jsonFile << "\t\t\t\t\t\t\t\"offset\": null,\n";
			else
				jsonFile << "\t\t\t\t\t\t\t\"offset\": " << structMember.offset << ",\n";

			if (structMember.size == msl::unknown)
				jsonFile << "\t\t\t\t\t\t\t\"size\": null,\n";
			else
				jsonFile << "\t\t\t\t\t\t\t\"size\": " << structMember.size << ",\n";

			auto type = static_cast<unsigned int>(structMember.type);
			if (type < msl::typeCount)
				jsonFile << "\t\t\t\t\t\t\t\"type\": \"" << typeNames[type] << "\",\n";
			else
				jsonFile << "\t\t\t\t\t\t\t\"type\": \"invalid\",\n";

			if (structMember.type == msl::Type::Struct)
			{
				jsonFile << "\t\t\t\t\t\t\t\"structIndex\": " << structMember.structIndex <<
					",\n";
			}

			jsonFile << "\t\t\t\t\t\t\t\"arrayElements\":\n\t\t\t\t\t\t\t[\n";
			for (uint32_t l = 0; l < structMember.arrayElementCount; ++l)
			{
				jsonFile << "\t\t\t\t\t\t\t\t{\n";
				msl::ArrayInfo arrayInfo;
				module.structMemberArrayInfo(arrayInfo, i, j, k, l);
				jsonFile << "\t\t\t\t\t\t\t\t\t\"length\": " << arrayInfo.length << ",\n";

				if (arrayInfo.stride == msl::unknown)
					jsonFile << "\t\t\t\t\t\t\t\t\t\"stride\": null\n";
				else
					jsonFile << "\t\t\t\t\t\t\t\t\t\"stride\": " << arrayInfo.stride << "\n";

				if (l == structMember.arrayElementCount - 1)
					jsonFile << "\t\t\t\t\t\t\t\t}\n";
				else
					jsonFile << "\t\t\t\t\t\t\t\t},\n";
			}
			jsonFile << "\t\t\t\t\t\t\t],\n";

			jsonFile << "\t\t\t\t\t\t\t\"rowMajor\": " <<
				(structMember.rowMajor ? "true" : "false") << "\n";

			if (k == pipelineStruct.memberCount - 1)
				jsonFile << "\t\t\t\t\t\t}\n";
			else
				jsonFile << "\t\t\t\t\t\t},\n";
		}
		jsonFile << "\t\t\t\t\t]\n";

		if (j == pipeline.structCount - 1)
			jsonFile << "\t\t\t\t}\n";
		else
			jsonFile << "\t\t\t\t},\n";
	}
	jsonFile << "\t\t\t],\n";
}

static void writeSamplerStates(std::ostream& jsonFile, const msl::Module& module,
	const msl::Pipeline& pipeline, uint32_t i)
{
	jsonFile << "\t\t\t\"samplerStates\":\n\t\t\t[\n";
	for (uint32_t j = 0; j < pipeline.samplerStateCount; ++j)
	{
		jsonFile << "\t\t\t\t{\n";
		msl::SamplerState samplerState;
		module.samplerState(samplerState, i, j);

		auto minFilter = static_cast<unsigned int>(samplerState.minFilter);
		if (minFilter < sizeof(filterNames)/sizeof(*filterNames))
			jsonFile << "\t\t\t\t\t\"minFilter\": \"" << filterNames[minFilter] << "\",\n";
		else
			jsonFile << "\t\t\t\t\t\"minFilter\": null,\n";

		auto magFilter = static_cast<unsigned int>(samplerState.magFilter);
		if (magFilter < sizeof(filterNames)/sizeof(*filterNames))
			jsonFile << "\t\t\t\t\t\"magFilter\": \"" << filterNames[magFilter] << "\",\n";
		else
			jsonFile << "\t\t\t\t\t\"magFilter\": null,\n";

		auto mipFilter = static_cast<unsigned int>(samplerState.mipFilter);
		if (mipFilter < sizeof(mipFilterNames)/sizeof(*mipFilterNames))
			jsonFile << "\t\t\t\t\t\"mipFilter\": \"" << mipFilterNames[mipFilter] << "\",\n";
		else
			jsonFile << "\t\t\t\t\t\"mipFilter\": null,\n";

		auto addressModeU = static_cast<unsigned int>(samplerState.addressModeU);
		if (addressModeU < sizeof(addressModeNames)/sizeof(*addressModeNames))
		{
			jsonFile << "\t\t\t\t\t\"addressModeU\": \"" << addressModeNames[addressModeU] <<
				"\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\"addressModeU\": null,\n";

		auto addressModeV = static_cast<unsigned int>(samplerState.addressModeV);
		if (addressModeV < sizeof(addressModeNames)/sizeof(*addressModeNames))
		{
			jsonFile << "\t\t\t\t\t\"addressModeV\": \"" << addressModeNames[addressModeV] <<
				"\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\"addressModeV\": null,\n";

		auto addressModeW = static_cast<unsigned int>(samplerState.addressModeW);
		if (addressModeW < sizeof(addressModeNames)/sizeof(*addressModeNames))
		{
			jsonFile << "\t\t\t\t\t\"addressModeW\": \"" << addressModeNames[addressModeW] <<
				"\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\"addressModeW\": null,\n";

		if (samplerState.mipLodBias == msl::unknownFloat)
			jsonFile << "\t\t\t\t\t\"mipLodBias\": null,\n";
		else
			jsonFile << "\t\t\t\t\t\"mipLodBias\": \"" << samplerState.mipLodBias << "\",\n";

		if (samplerState.maxAnisotropy == msl::unknownFloat)
			jsonFile << "\t\t\t\t\t\"maxAnisotropy\": null,\n";
		else
		{
			jsonFile << "\t\t\t\t\t\"maxAnisotropy\": \"" << samplerState.maxAnisotropy <<
				"\",\n";
		}

		if (samplerState.minLod == msl::unknownFloat)
			jsonFile << "\t\t\t\t\t\"minLod\": null,\n";
		else
			jsonFile << "\t\t\t\t\t\"minLod\": \"" << samplerState.minLod << "\",\n";

		if (samplerState.maxLod == msl::unknownFloat)
			jsonFile << "\t\t\t\t\t\"maxLod\": null,\n";
		else
			jsonFile << "\t\t\t\t\t\"maxLod\": \"" << samplerState.maxLod << "\",\n";

		auto borderColor = static_cast<unsigned int>(samplerState.borderColor);
		if (borderColor < sizeof(borderColorNames)/sizeof(*borderColorNames))
		{
			jsonFile << "\t\t\t\t\t\"borderColor\": \"" << borderColorNames[borderColor] <<
				"\"\n";
		}
		else
			jsonFile << "\t\t\t\t\t\"borderColor\": null\n";

		auto compareOp = static_cast<unsigned int>(samplerState.compareOp);
		if (compareOp < sizeof(compareOpNames)/sizeof(*compareOpNames))
		{
			jsonFile << "\t\t\t\t\t\"compareOp\": \"" << compareOpNames[compareOp] <<
				"\"\n";
		}
		else
			jsonFile << "\t\t\t\t\t\"compareOp\": null\n";

		if (j == pipeline.samplerStateCount - 1)
			jsonFile << "\t\t\t\t}\n";
		else
			jsonFile << "\t\t\t\t},\n";
	}
	jsonFile << "\t\t\t],\n";
}

static void writeUniforms(std::ostream& jsonFile, const msl::Module& module,
	const msl::Pipeline& pipeline, uint32_t i)
{
	jsonFile << "\t\t\t\"uniforms\":\n\t\t\t[\n";
	for (uint32_t j = 0; j < pipeline.uniformCount; ++j)
	{
		jsonFile << "\t\t\t\t{\n";
		msl::Uniform uniform;
		module.uniform(uniform, i, j);

		jsonFile << "\t\t\t\t\t\"name\": \"" << uniform.name << "\",\n";

		auto uniformType = static_cast<unsigned int>(uniform.uniformType);
		if (uniformType < sizeof(uniformTypeNames)/sizeof(*uniformTypeNames))
		{
			jsonFile << "\t\t\t\t\t\"uniformType\": \"" << uniformTypeNames[uniformType] <<
				"\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\"uniformType\": \"invalid\",\n";

		auto type = static_cast<unsigned int>(uniform.type);
		if (type < msl::typeCount)
			jsonFile << "\t\t\t\t\t\"type\": \"" << typeNames[type] << "\",\n";
		else
			jsonFile << "\t\t\t\t\t\"type\": \"invalid\",\n";

		if (uniform.type == msl::Type::Struct)
			jsonFile << "\t\t\t\t\t\"structIndex\": " << uniform.structIndex << ",\n";

		jsonFile << "\t\t\t\t\t\"arrayElements\":\n\t\t\t\t\t[\n";
		for (uint32_t k = 0; k < uniform.arrayElementCount; ++k)
		{
			jsonFile << "\t\t\t\t\t\t{\n";
			msl::ArrayInfo arrayInfo;
			module.uniformArrayInfo(arrayInfo, i, j, k);
			jsonFile << "\t\t\t\t\t\t\t\"length\": " << arrayInfo.length << ",\n";

			if (arrayInfo.stride == msl::unknown)
				jsonFile << "\t\t\t\t\t\t\t\"stride\": null\n";
			else
				jsonFile << "\t\t\t\t\t\t\t\"stride\": " << arrayInfo.stride << "\n";

			if (k == uniform.arrayElementCount - 1)
				jsonFile << "\t\t\t\t\t\t\t\t}\n";
			else
				jsonFile << "\t\t\t\t\t\t\t\t},\n";
		}
		jsonFile << "\t\t\t\t\t],\n";

		if (uniform.descriptorSet == msl::unknown)
			jsonFile << "\t\t\t\t\t\"descriptorSet\": null,\n";
		else
			jsonFile << "\t\t\t\t\t\"descriptorSet\": " << uniform.descriptorSet << ",\n";

		if (uniform.binding == msl::unknown)
			jsonFile << "\t\t\t\t\t\"binding\": null,\n";
		else
			jsonFile << "\t\t\t\t\t\"binding\": " << uniform.binding << ",\n";

		if (uniform.samplerIndex == msl::unknown)
			jsonFile << "\t\t\t\t\t\"samplerIndex\": null\n";
		else
			jsonFile << "\t\t\t\t\t\"samplerIndex\": " << uniform.samplerIndex << "\n";

		if (j == pipeline.uniformCount - 1)
			jsonFile << "\t\t\t\t}\n";
		else
			jsonFile << "\t\t\t\t},\n";
	}
	jsonFile << "\t\t\t],\n";

	if (pipeline.pushConstantStruct == msl::unknown)
		jsonFile << "\t\t\t\"pushConstantStruct\": null,\n";
	else
		jsonFile << "\t\t\t\"pushConstantStruct\": " << pipeline.pushConstantStruct << ",\n";
}

static void writeAttributes(std::ostream& jsonFile, const msl::Module& module,
	const msl::Pipeline& pipeline, uint32_t i)
{
	jsonFile << "\t\t\t\"attributes\":\n\t\t\t[\n";
	for (uint32_t j = 0; j < pipeline.attributeCount; ++j)
	{
		jsonFile << "\t\t\t\t{\n";
		msl::Attribute attribute;
		module.attribute(attribute, i, j);

		jsonFile << "\t\t\t\t\t\"name\": \"" << attribute.name << "\",\n";

		auto type = static_cast<unsigned int>(attribute.type);
		if (type < msl::typeCount)
			jsonFile << "\t\t\t\t\t\"type\": \"" << typeNames[type] << "\",\n";
		else
			jsonFile << "\t\t\t\t\t\"type\": \"invalid\",\n";

		jsonFile << "\t\t\t\t\t\"arrayElements\":\n\t\t\t\t\t[\n";
		for (uint32_t k = 0; k < attribute.arrayElementCount; ++k)
		{
			jsonFile << "\t\t\t\t\t\t{\n";

			jsonFile << "\t\t\t\t\t\t\t\"length\": " << module.attributeArrayLength(i, j, k)
				<< "\n";

			if (k == attribute.arrayElementCount - 1)
				jsonFile << "\t\t\t\t\t\t}\n";
			else
				jsonFile << "\t\t\t\t\t\t},\n";
		}
		jsonFile << "\t\t\t\t\t],\n";

		jsonFile << "\t\t\t\t\t\"location\": " << attribute.location << ",\n";
		jsonFile << "\t\t\t\t\t\"component\": " << attribute.component << "\n";

		if (j == pipeline.attributeCount - 1)
			jsonFile << "\t\t\t\t}\n";
		else
			jsonFile << "\t\t\t\t},\n";
	}
	jsonFile << "\t\t\t],\n";
}

static void writeFragmentOutputs(std::ostream& jsonFile, const msl::Module& module,
	const msl::Pipeline& pipeline, uint32_t i)
{
	jsonFile << "\t\t\t\"fragmentOutputs\":\n\t\t\t[\n";
	for (uint32_t j = 0; j < pipeline.fragmentOutputCount; ++j)
	{
		jsonFile << "\t\t\t\t{\n";
		msl::FragmentOutput fragmentOutput;
		module.fragmentOutput(fragmentOutput, i, j);

		jsonFile << "\t\t\t\t\t\"name\": \"" << fragmentOutput.name << "\",\n";
		jsonFile << "\t\t\t\t\t\"location\": " << fragmentOutput.location << "\n";

		if (j == pipeline.fragmentOutputCount - 1)
			jsonFile << "\t\t\t\t}\n";
		else
			jsonFile << "\t\t\t\t},\n";
	}
	jsonFile << "\t\t\t],\n";
}

static void writeRasterizationState(std::ostream& jsonFile,
	const msl::RasterizationState& rasterizationState)
{
	jsonFile << "\t\t\t\t\"rasterizationState\":\n\t\t\t\t{\n";

	auto depthClampEnable = static_cast<unsigned int>(rasterizationState.depthClampEnable);
	if (depthClampEnable < sizeof(boolNames)/sizeof(*boolNames))
		jsonFile << "\t\t\t\t\t\"depthClampEnable\": " << boolNames[depthClampEnable] << ",\n";
	else
		jsonFile << "\t\t\t\t\t\"depthClampEnable\": null,\n";

	auto rasterizerDiscardEnable =
		static_cast<unsigned int>(rasterizationState.rasterizerDiscardEnable);
	if (rasterizerDiscardEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"rasterizerDiscardEnable\": " <<
			boolNames[rasterizerDiscardEnable] << ",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"rasterizerDiscardEnable\": null,\n";

	auto polygonMode = static_cast<unsigned int>(rasterizationState.polygonMode);
	if (polygonMode < sizeof(polygonModeNames)/sizeof(*polygonModeNames))
		jsonFile << "\t\t\t\t\t\"polygonMode\": \"" << polygonModeNames[polygonMode] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\"polygonMode\": null,\n";

	auto cullMode = static_cast<unsigned int>(rasterizationState.cullMode);
	if (cullMode < sizeof(cullModeNames)/sizeof(*cullModeNames))
		jsonFile << "\t\t\t\t\t\"cullMode\": \"" << cullModeNames[cullMode] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\"cullMode\": null,\n";

	auto frontFace = static_cast<unsigned int>(rasterizationState.frontFace);
	if (frontFace < sizeof(frontFaceNames)/sizeof(*frontFaceNames))
		jsonFile << "\t\t\t\t\t\"frontFace\": \"" << frontFaceNames[frontFace] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\"frontFace\": null,\n";

	auto depthBiasEnable =
		static_cast<unsigned int>(rasterizationState.depthBiasEnable);
	if (depthBiasEnable < sizeof(boolNames)/sizeof(*boolNames))
		jsonFile << "\t\t\t\t\t\"depthBiasEnable\": " << boolNames[depthBiasEnable] << ",\n";
	else
		jsonFile << "\t\t\t\t\t\"depthBiasEnable\": null,\n";

	if (rasterizationState.depthBiasConstantFactor == msl::unknownFloat)
		jsonFile << "\t\t\t\t\t\"depthBiasEnable\": null,\n";
	else
	{
		jsonFile << "\t\t\t\t\t\"depthBiasConstantFactor\": " <<
			rasterizationState.depthBiasConstantFactor << ",\n";
	}

	if (rasterizationState.depthBiasClamp == msl::unknownFloat)
		jsonFile << "\t\t\t\t\t\"depthBiasClamp\": null,\n";
	else
	{
		jsonFile << "\t\t\t\t\t\"depthBiasClamp\": " <<
			rasterizationState.depthBiasClamp << ",\n";
	}

	if (rasterizationState.depthBiasSlopeFactor == msl::unknownFloat)
		jsonFile << "\t\t\t\t\t\"depthBiasSlopeFactor\": null,\n";
	else
	{
		jsonFile << "\t\t\t\t\t\"depthBiasSlopeFactor\": " <<
			rasterizationState.depthBiasSlopeFactor << ",\n";
	}

	if (rasterizationState.lineWidth == msl::unknownFloat)
		jsonFile << "\t\t\t\t\t\"lineWidth\": null\n";
	else
		jsonFile << "\t\t\t\t\t\"lineWidth\": " << rasterizationState.lineWidth << "\n";

	jsonFile << "\t\t\t\t},\n";
}

static void writeMultisampleState(std::ostream& jsonFile,
	const msl::MultisampleState& multisampleState)
{
	jsonFile << "\t\t\t\t\"multisampleState\":\n\t\t\t\t{\n";

	auto sampleShadingEnable =
		static_cast<unsigned int>(multisampleState.sampleShadingEnable);
	if (sampleShadingEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"sampleShadingEnable\": " << boolNames[sampleShadingEnable] <<
			",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"sampleShadingEnable\": null,\n";

	if (multisampleState.minSampleShading == msl::unknownFloat)
		jsonFile << "\t\t\t\t\t\"minSampleShading\": null,\n";
	else
	{
		jsonFile << "\t\t\t\t\t\"minSampleShading\": " <<
			multisampleState.minSampleShading << ",\n";
	}

	if (multisampleState.sampleMask == msl::unknown)
		jsonFile << "\t\t\t\t\t\"sampleMask\": null,\n";
	else
		jsonFile << "\t\t\t\t\t\"sampleMask\": " << multisampleState.sampleMask << ",\n";

	auto alphaToCoverageEnable =
		static_cast<unsigned int>(multisampleState.alphaToCoverageEnable);
	if (alphaToCoverageEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"alphaToCoverageEnable\": " <<
			boolNames[alphaToCoverageEnable] << ",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"alphaToCoverageEnable\": null,\n";

	auto alphaToOneEnable =
		static_cast<unsigned int>(multisampleState.alphaToOneEnable);
	if (alphaToOneEnable < sizeof(boolNames)/sizeof(*boolNames))
		jsonFile << "\t\t\t\t\t\"alphaToOneEnable\": " << boolNames[alphaToOneEnable] << "\n";
	else
		jsonFile << "\t\t\t\t\t\"alphaToOneEnable\": null\n";

	jsonFile << "\t\t\t\t},\n";
}

static void writeDepthStencilOpState(std::ostream& jsonFile,
	const msl::StencilOpState& stencilOpState)
{
	auto failOp = static_cast<unsigned int>(stencilOpState.failOp);
	if (failOp < sizeof(stencilOpNames)/sizeof(*stencilOpNames))
		jsonFile << "\t\t\t\t\t\t\"failOp\": \"" << stencilOpNames[failOp] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\t\"failOp\": null,\n";

	auto passOp = static_cast<unsigned int>(stencilOpState.passOp);
	if (passOp < sizeof(stencilOpNames)/sizeof(*stencilOpNames))
		jsonFile << "\t\t\t\t\t\t\"passOp\": \"" << stencilOpNames[passOp] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\t\"passOp\": null,\n";

	auto depthFailOp = static_cast<unsigned int>(stencilOpState.depthFailOp);
	if (depthFailOp < sizeof(stencilOpNames)/sizeof(*stencilOpNames))
		jsonFile << "\t\t\t\t\t\t\"depthFailOp\": \"" << stencilOpNames[depthFailOp] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\t\"depthFailOp\": null,\n";

	auto compareOp = static_cast<unsigned int>(stencilOpState.compareOp);
	if (compareOp < sizeof(compareOpNames)/sizeof(*compareOpNames))
		jsonFile << "\t\t\t\t\t\t\"compareOp\": \"" << compareOpNames[compareOp] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\t\"compareOp\": null,\n";

	if (stencilOpState.compareMask == msl::unknown)
		jsonFile << "\t\t\t\t\t\t\"compareMask\": null,\n";
	else
		jsonFile << "\t\t\t\t\t\t\"compareMask\": " << stencilOpState.compareMask << ",\n";

	if (stencilOpState.writeMask == msl::unknown)
		jsonFile << "\t\t\t\t\t\t\"writeMask\": null,\n";
	else
		jsonFile << "\t\t\t\t\t\t\"writeMask\": " << stencilOpState.writeMask << ",\n";

	if (stencilOpState.reference == msl::unknown)
		jsonFile << "\t\t\t\t\t\t\"reference\": null\n";
	else
		jsonFile << "\t\t\t\t\t\t\"reference\": " << stencilOpState.reference << "\n";
}

static void writeDepthStencilState(std::ostream& jsonFile,
	const msl::DepthStencilState& depthStencilState)
{
	jsonFile << "\t\t\t\t\"depthStencilState\":\n\t\t\t\t{\n";

	auto depthTestEnable =
		static_cast<unsigned int>(depthStencilState.depthTestEnable);
	if (depthTestEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"depthTestEnable\": " << boolNames[depthTestEnable] <<
			",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"depthTestEnable\": null,\n";

	auto depthWriteEnable =
		static_cast<unsigned int>(depthStencilState.depthWriteEnable);
	if (depthWriteEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"depthWriteEnable\": " << boolNames[depthWriteEnable] <<
			",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"depthWriteEnable\": null,\n";

	auto depthCompareOp =
		static_cast<unsigned int>(depthStencilState.depthCompareOp);
	if (depthCompareOp < sizeof(compareOpNames)/sizeof(*compareOpNames))
		jsonFile << "\t\t\t\t\t\"depthCompareOp\": \"" << compareOpNames[depthCompareOp] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\"depthCompareOp\": null,\n";

	auto depthBoundsTestEnable =
		static_cast<unsigned int>(depthStencilState.depthBoundsTestEnable);
	if (depthBoundsTestEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"depthBoundsTestEnable\": " <<
			boolNames[depthBoundsTestEnable] << ",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"depthBoundsTestEnable\": null,\n";

	auto stencilTestEnable =
		static_cast<unsigned int>(depthStencilState.stencilTestEnable);
	if (stencilTestEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"stencilTestEnable\": " << boolNames[stencilTestEnable] <<
			",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"stencilTestEnable\": null,\n";

	jsonFile << "\t\t\t\t\t\"frontStencil\":\n\t\t\t\t\t{\n";
	writeDepthStencilOpState(jsonFile, depthStencilState.frontStencil);
	jsonFile << "\t\t\t\t\t},\n";

	jsonFile << "\t\t\t\t\t\"backStencil\":\n\t\t\t\t\t{\n";
	writeDepthStencilOpState(jsonFile, depthStencilState.backStencil);
	jsonFile << "\t\t\t\t\t},\n";

	if (depthStencilState.minDepthBounds == msl::unknownFloat)
		jsonFile << "\t\t\t\t\t\"minDepthBounds\": null,\n";
	else
	{
		jsonFile << "\t\t\t\t\t\"minDepthBounds\": " <<
			depthStencilState.minDepthBounds << ",\n";
	}

	if (depthStencilState.maxDepthBounds == msl::unknownFloat)
		jsonFile << "\t\t\t\t\t\"maxDepthBounds\": null\n";
	else
	{
		jsonFile << "\t\t\t\t\t\"maxDepthBounds\": " <<
			depthStencilState.maxDepthBounds << "\n";
	}

	jsonFile << "\t\t\t\t},\n";
}

static void writeBlendState(std::ostream& jsonFile, const msl::BlendState& blendState)
{
	jsonFile << "\t\t\t\t\"blendState\":\n\t\t\t\t{\n";

	auto logicalOpEnable =
		static_cast<unsigned int>(blendState.logicalOpEnable);
	if (logicalOpEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"logicalOpEnable\": " << boolNames[logicalOpEnable] <<
			",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"logicalOpEnable\": null,\n";

	auto logicalOp =
		static_cast<unsigned int>(blendState.logicalOp);
	if (logicalOp < sizeof(logicOpNames)/sizeof(*logicOpNames))
		jsonFile << "\t\t\t\t\t\"logicalOp\": \"" << logicOpNames[logicalOp] << "\",\n";
	else
		jsonFile << "\t\t\t\t\t\"logicalOp\": null,\n";

	auto separateAttachmentBlendingEnable =
		static_cast<unsigned int>(blendState.separateAttachmentBlendingEnable);
	if (separateAttachmentBlendingEnable < sizeof(boolNames)/sizeof(*boolNames))
	{
		jsonFile << "\t\t\t\t\t\"separateAttachmentBlendingEnable\": " <<
			boolNames[separateAttachmentBlendingEnable] << ",\n";
	}
	else
		jsonFile << "\t\t\t\t\t\"separateAttachmentBlendingEnable\": null,\n";

	jsonFile << "\t\t\t\t\t\"blendAttachments\":\n\t\t\t\t\t[\n";

	for (unsigned int i = 0; i < msl::maxAttachments; ++i)
	{
		jsonFile << "\t\t\t\t\t\t{\n";

		auto blendEnable =
			static_cast<unsigned int>(blendState.blendAttachments[i].blendEnable);
		if (blendEnable < sizeof(boolNames)/sizeof(*boolNames))
		{
			jsonFile << "\t\t\t\t\t\t\t\"blendEnable\": " << boolNames[blendEnable] <<
				",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\t\t\"blendEnable\": null,\n";

		auto srcColorBlendFactor =
			static_cast<unsigned int>(blendState.blendAttachments[i].srcColorBlendFactor);
		if (srcColorBlendFactor < sizeof(blendFactorNames)/sizeof(*blendFactorNames))
		{
			jsonFile << "\t\t\t\t\t\t\t\"srcColorBlendFactor\": \"" <<
				blendFactorNames[srcColorBlendFactor] << "\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\t\t\"srcColorBlendFactor\": null,\n";

		auto dstColorBlendFactor =
			static_cast<unsigned int>(blendState.blendAttachments[i].dstColorBlendFactor);
		if (dstColorBlendFactor < sizeof(blendFactorNames)/sizeof(*blendFactorNames))
		{
			jsonFile << "\t\t\t\t\t\t\t\"dstColorBlendFactor\": \"" <<
				blendFactorNames[dstColorBlendFactor] << "\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\t\t\"dstColorBlendFactor\": null,\n";

		auto colorBlendOp =
			static_cast<unsigned int>(blendState.blendAttachments[i].colorBlendOp);
		if (colorBlendOp < sizeof(blendOpNames)/sizeof(*blendOpNames))
		{
			jsonFile << "\t\t\t\t\t\t\t\"colorBlendOp\": \"" <<
				blendOpNames[colorBlendOp] << "\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\t\t\"colorBlendOp\": null,\n";

		auto srcAlphaBlendFactor =
			static_cast<unsigned int>(blendState.blendAttachments[i].srcAlphaBlendFactor);
		if (srcAlphaBlendFactor < sizeof(blendFactorNames)/sizeof(*blendFactorNames))
		{
			jsonFile << "\t\t\t\t\t\t\t\"srcAlphaBlendFactor\": \"" <<
				blendFactorNames[srcAlphaBlendFactor] << "\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\t\t\"srcAlphaBlendFactor\": null,\n";

		auto dstAlphaBlendFactor =
			static_cast<unsigned int>(blendState.blendAttachments[i].dstAlphaBlendFactor);
		if (dstAlphaBlendFactor < sizeof(blendFactorNames)/sizeof(*blendFactorNames))
		{
			jsonFile << "\t\t\t\t\t\t\t\"dstAlphaBlendFactor\": \"" <<
				blendFactorNames[dstAlphaBlendFactor] << "\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\t\t\"dstAlphaBlendFactor\": null,\n";

		auto alphaBlendOp =
			static_cast<unsigned int>(blendState.blendAttachments[i].alphaBlendOp);
		if (alphaBlendOp < sizeof(blendOpNames)/sizeof(*blendOpNames))
		{
			jsonFile << "\t\t\t\t\t\t\t\"alphaBlendOp\": \"" <<
				blendOpNames[alphaBlendOp] << "\",\n";
		}
		else
			jsonFile << "\t\t\t\t\t\t\t\"alphaBlendOp\": null,\n";

		if (blendState.blendAttachments[i].colorWriteMask == msl::ColorMaskUnset)
			jsonFile << "\t\t\t\t\t\t\t\"colorWriteMask\": null\n";
		else
		{
			jsonFile << "\t\t\t\t\t\t\t\"colorWriteMask\": ";

			if (blendState.blendAttachments[i].colorWriteMask == msl::ColorMaskNone)
				jsonFile << 0;
			else
			{
				jsonFile << "\"";
				if (blendState.blendAttachments[i].colorWriteMask & msl::ColorMaskRed)
					jsonFile << "R";
				if (blendState.blendAttachments[i].colorWriteMask & msl::ColorMaskGreen)
					jsonFile << "G";
				if (blendState.blendAttachments[i].colorWriteMask & msl::ColorMaskBlue)
					jsonFile << "B";
				if (blendState.blendAttachments[i].colorWriteMask & msl::ColorMaskAlpha)
					jsonFile << "A";
				jsonFile << "\"";
			}

			jsonFile << "\n";
		}

		if (i == msl::maxAttachments - 1)
			jsonFile << "\t\t\t\t\t\t}\n";
		else
			jsonFile << "\t\t\t\t\t\t},\n";
	}

	jsonFile << "\t\t\t\t\t],\n";

	jsonFile << "\t\t\t\t\t\"blendConstants\": {\"r\": ";
	if (blendState.blendConstants[0] == msl::unknownFloat)
		jsonFile << "null";
	else
		jsonFile << blendState.blendConstants[0];
	jsonFile << ", \"g\": ";
	if (blendState.blendConstants[1] == msl::unknownFloat)
		jsonFile << "null";
	else
		jsonFile << blendState.blendConstants[1];
	jsonFile << ", \"b\": ";
	if (blendState.blendConstants[2] == msl::unknownFloat)
		jsonFile << "null";
	else
		jsonFile << blendState.blendConstants[2];
	jsonFile << ", \"a\": ";
	if (blendState.blendConstants[3] == msl::unknownFloat)
		jsonFile << "null";
	else
		jsonFile << blendState.blendConstants[3];
	jsonFile << "}\n";

	jsonFile << "\t\t\t\t},\n";

}

static void writeRenderState(std::ostream& jsonFile, const msl::Module& module, uint32_t i)
{
	jsonFile << "\t\t\t\"renderState\":\n\t\t\t{\n";
	msl::RenderState renderState;
	module.renderState(renderState, i);

	writeRasterizationState(jsonFile, renderState.rasterizationState);
	writeMultisampleState(jsonFile, renderState.multisampleState);
	writeDepthStencilState(jsonFile, renderState.depthStencilState);
	writeBlendState(jsonFile, renderState.blendState);

	if (renderState.patchControlPoints == msl::unknown)
		jsonFile << "\t\t\t\t\"patchControlPoints\": null,\n";
	else
		jsonFile << "\t\t\t\t\"patchControlPoints\": " << renderState.patchControlPoints << ",\n";

	jsonFile << "\t\t\t\t\"clipDistanceCount\": " << renderState.clipDistanceCount << ",\n";
	jsonFile << "\t\t\t\t\"cullDistanceCount\": " << renderState.cullDistanceCount << "\n";
	jsonFile << "\t\t\t},\n";
}

static void writeComputeLocalSize(std::ostream& jsonFile, const msl::Pipeline& pipeline)
{
	jsonFile << "\t\t\t\"computeLocalSize\": [" << pipeline.computeLocalSize[0] << ", " <<
		pipeline.computeLocalSize[1] << ", " << pipeline.computeLocalSize[2] << "]\n";
}

int main(int argc, char** argv)
{
	// Specify the options.
	options_description mainOptions("options");
	mainOptions.add_options()
		("help,h", "display this help message")
		("version,v", "print the version number and exit")
		("input,i", value<std::string>()->required(),
			"input shader module file to extract")
		("output,o", value<std::string>()->required(), "output directory to extract to. This will "
			"be created if it doesn't exist.");

	positional_options_description positionalOptions;
	positionalOptions.add("input", 1);

	int exitCode = 0;
	variables_map options;
	try
	{
		store(command_line_parser(argc, argv).
			options(mainOptions).positional(positionalOptions).run(), options);
		notify(options);
	}
	catch (std::exception& e)
	{
		if (!options.count("help") && !options.count("version"))
		{
			std::cerr << "error: " << e.what() << std::endl << std::endl;
			exitCode = 1;
		}
	}

	if (options.count("help") || exitCode != 0)
	{
		std::cout << "Usage: mslb-extract -o output file" << std::endl << std::endl;
		std::cout << "Version " << MSL_MAJOR_VERSION << "." << MSL_MINOR_VERSION << "." <<
			MSL_PATCH_VERSION << std::endl;
		std::cout << "Extract a compiled shader module into its components." << std::endl <<
			std::endl;
		std::cout <<
			"The shader for each pipeline stage will be written to the output directory. The\n"
			"name will be based on the module name, pipelien name, and have an extension \n"
			"based on the stage.\n"
			"    <module>.<pipeline>.vert\n"
			"    <module>.<pipeline>.tesc\n"
			"    <module>.<pipeline>.tese\n"
			"    <module>.<pipeline>.frag\n"
			"    <module>.<pipeline>.geom\n"
			"    <module>.<pipeline>.comp\n\n"
			"Unused stages will have no output file. The format of the file will depend on \n"
			"the target, and may either be text or binary.\n\n"
			"Additionally, the following two files will be output:\n"
			"    <module>.json: json file describing each pipeline in the module.\n"
			"    <module>.shared: the shared data (only for certain targets)" <<
			std::endl << std::endl;

		std::cout << mainOptions;
		return exitCode;
	}
	else if (options.count("version"))
	{
		std::cout << "mslb-extract version " << MSL_MAJOR_VERSION << "." << MSL_MINOR_VERSION <<
			"." << MSL_PATCH_VERSION << std::endl;
		return exitCode;
	}

	msl::Module module;
	std::string moduleFile = options["input"].as<std::string>();
	if (!module.read(moduleFile))
	{
		std::cerr << "error: could not read shader module: " << moduleFile << std::endl;
		return 2;
	}

	path outputDir = options["output"].as<std::string>();
	boost::system::error_code ec;
	create_directories(outputDir, ec);
	if (ec)
	{
		std::cerr << "error: could not create directory: " << outputDir.string() << std::endl;
		return 3;
	}

	// Write out json description.
	std::string moduleName = path(moduleFile).filename().replace_extension().string();
	std::string jsonFileName = (outputDir/(moduleName + ".json")).string();
	std::ofstream jsonFile;
	if (!openFile(jsonFile, jsonFileName, false))
		return 3;

	uint32_t targetId = module.targetId();
	jsonFile << "{\n";

	// Target info
	jsonFile << "\t\"targetId\": \"" <<
		static_cast<char>(targetId >> 24) <<
		static_cast<char>(targetId >> 16) <<
		static_cast<char>(targetId >> 8) <<
		static_cast<char>(targetId) << "\",\n";
	jsonFile << "\t\"targetVersion\": " << module.targetVersion() << ",\n";

	// Pipelines
	jsonFile << "\t\"pipelines\":\n\t[\n";
	bool textShaders = shadersAreText(module);
	uint32_t pipelineCount = module.pipelineCount();
	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		jsonFile << "\t\t{\n";

		msl::Pipeline pipeline;
		module.pipeline(pipeline, i);
		jsonFile << "\t\t\t\"name\": \"" << pipeline.name << "\",\n";

		// Stage shaders
		for (unsigned int j = 0; j < msl::stageCount; ++j)
		{
			uint32_t shader = pipeline.shaders[j];
			if (shader == msl::unknown)
				continue;

			std::string shaderName = moduleName + "." + pipeline.name + stageExtensions[j];
			jsonFile << "\t\t\t\"" << stageNames[j] << "\": \"" << shaderName << "\",\n";

			std::string shaderFileName = (outputDir/shaderName).string();
			std::ofstream shaderStream;
			if (!openFile(shaderStream, shaderFileName, !textShaders))
				return 3;

			uint32_t writeSize = module.shaderSize(shader);
			// Remove null terminator for text shaders.
			if (writeSize > 0 && textShaders)
				--writeSize;
			shaderStream.write(reinterpret_cast<const char*>(module.shaderData(shader)), writeSize);
		}

		writeStructs(jsonFile, module, pipeline, i);
		writeSamplerStates(jsonFile, module, pipeline, i);
		writeUniforms(jsonFile, module, pipeline, i);
		writeAttributes(jsonFile, module, pipeline, i);
		writeFragmentOutputs(jsonFile, module, pipeline, i);
		writeRenderState(jsonFile, module, i);
		writeComputeLocalSize(jsonFile, pipeline);

		if (i == pipelineCount - 1)
			jsonFile << "\t\t}\n";
		else
			jsonFile << "\t\t},\n";
	}


	uint32_t sharedDataSize = module.sharedDataSize();
	if (sharedDataSize > 0)
	{
		jsonFile << "\t],\n";
		std::string sharedName = moduleName + ".shared";
		jsonFile << "\t\"sharedData\": \"" << sharedName << "\"\n}\n";

		std::string sharedFileName = (outputDir/sharedName).string();
		std::ofstream sharedStream;
		if (!openFile(sharedStream, sharedFileName, false))
			return 3;

		sharedStream.write(reinterpret_cast<const char*>(module.sharedData()), sharedDataSize);
	}
	else
		jsonFile << "\t]\n}\n";

	std::cout << "extracted module contents to " << outputDir.string() << std::endl;
	return 0;
}
