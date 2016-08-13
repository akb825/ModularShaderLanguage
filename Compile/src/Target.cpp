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

#include <MSL/Compile/Target.h>
#include <MSL/Compile/CompiledResult.h>
#include <MSL/Compile/Output.h>
#include "Compiler.h"
#include "ExecuteCommand.h"
#include "Parser.h"
#include "Preprocessor.h"
#include "glslang/MachineIndependent/gl_types.h"
#include "StandAlone/ResourceLimits.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace msl
{

static Target::FeatureInfo featureInfos[] =
{
	// Types
	{"Integers", "HAS_INTEGERS", "Integer types."},
	{"Doubles", "HAS_DOUBLES", "Double types."},
	{"NonSquareMatrices", "HAS_NON_SQUARE_MATRICES", "Non-square matrices, such as Mat3x4."},
	{"Texture3D", "HAS_TEXTURE3D", "3D textures."},
	{"TextureArray", "HAS_TEXTURE_ARRAY", "Texture arrays."},
	{"ShadowSamplers", "HAS_SHADOW_SAMPLERS", "Shadow texture samplers."},
	{"MultisampledTextures", "HAS_MULTISAMPLED_TEXTURES", "Multisampled texture samplers."},
	{"IntegerTextures", "HAS_INTEGER_TEXTURES", "Integer texture samplers."},
	{"Images", "HAS_IMAGES", "Image types."},

	// Storage
	{"UniformBuffers", "HAS_UNIFORM_BUFFERS",
		"Uniform buffers. If disabled, uniform buffers will be converted to push constants. "
		"(equivalent to individual uniforms)"},
	{"Buffers", "HAS_BUFFERS", "Generic buffer types."},
	{"Std140", "HAS_STD140", "std140 block layout."},
	{"Std430", "HAS_STD430", "std430 block layout."},

	// Pipeline stages
	{"TessellationStages", "HAS_TESSELLATION_STAGES",
		"Tessellation control and evaluation stages."},
	{"GeometryStage", "HAS_GEOMETRY_STAGE", "Geometry stage."},
	{"ComputeStage", "HAS_COMPUTE_STAGE", "Compute stage."},

	// Fragment shader controls
	{"MultipleRenderTargets", "HAS_MULTIPLE_RENDER_TARGETS",
		"Supports writing to more than one render target."},
	{"DualSourceBlending", "HAS_DUAL_SOURCE_BLENDING",
		"Supports outputting two colors to the same output."},
	{"DepthHints", "HAS_DEPTH_HINTS", "Allow hints to be givin about the depth output."},

	// Other functionality
	{"Derivatives", "HAS_DERIVATIVES", "dFdx() an dFdy() functions."},
	{"AdvancedDerivatives", "HAS_ADVANCED_DERIVATIVES",
		"Coarse and fine versions of dFdx() and dFdy()."},
	{"MemoryBarriers", "HAS_MEMORY_BARRIERS", "Memory barrier functions."},
	{"PrimitiveStreams", "HAS_PRIMITIVE_STREAMS", "Primitive streams for geometry shaders."},
	{"InterpolationFunctions", "HAS_INTERPOLATION_FUNCTIONS",
		"Interpolation functions for centroid, sample, and offset."},
	{"TextureGather", "HAS_TEXTURE_GATHER", "textureGather() functions."},
	{"TexelFetch", "HAS_TEXEL_FETCH", "texelFetch() functions."},
	{"TextureSize", "HAS_TEXTURE_SIZE", "textureSize() functions."},
	{"TextureQueryLod", "HAS_TEXTURE_QUERY_LOD", "texureQueryLod() functions."},
	{"TextureQueryLevels", "HAS_TEXTURE_QUERY_LEVELS", "textureQueryLevels() functions."},
	{"TextureSamples", "HAS_TEXTURE_SAMPLES", "textureSamples() functions."},
	{"BitFunctions", "HAS_BIT_FUNCTIONS",
		"Integer functions such as bitfieldInsert() and findMSB()."},
	{"PackingFunctions", "HAS_PACKING_FUNCTIONS",
		"Packing and unpacking functions such as packUnorm2x16()."}
};

static_assert(sizeof(featureInfos)/sizeof(*featureInfos) == Target::featureCount,
	"Not all features are in the featureInfos array.");

static std::unordered_map<int, CompiledResult::Type> typeMap =
{
	// Scalars and vectors
	{GL_FLOAT, CompiledResult::Type::Float},
	{GL_FLOAT_VEC2, CompiledResult::Type::Vec2},
	{GL_FLOAT_VEC3, CompiledResult::Type::Vec3},
	{GL_FLOAT_VEC4, CompiledResult::Type::Vec4},
	{GL_DOUBLE, CompiledResult::Type::Double},
	{GL_DOUBLE_VEC2, CompiledResult::Type::DVec2},
	{GL_DOUBLE_VEC3, CompiledResult::Type::DVec3},
	{GL_DOUBLE_VEC4, CompiledResult::Type::DVec4},
	{GL_INT, CompiledResult::Type::Int},
	{GL_INT_VEC2, CompiledResult::Type::IVec2},
	{GL_INT_VEC3, CompiledResult::Type::IVec3},
	{GL_INT_VEC4, CompiledResult::Type::IVec4},
	{GL_UNSIGNED_INT, CompiledResult::Type::UInt},
	{GL_UNSIGNED_INT_VEC2, CompiledResult::Type::UVec2},
	{GL_UNSIGNED_INT_VEC3, CompiledResult::Type::UVec3},
	{GL_UNSIGNED_INT_VEC4, CompiledResult::Type::UVec4},
	{GL_BOOL, CompiledResult::Type::Bool},
	{GL_BOOL_VEC2, CompiledResult::Type::BVec2},
	{GL_BOOL_VEC3, CompiledResult::Type::BVec3},
	{GL_BOOL_VEC4, CompiledResult::Type::BVec4},

	// Matrices
	{GL_FLOAT_MAT2, CompiledResult::Type::Mat2},
	{GL_FLOAT_MAT3, CompiledResult::Type::Mat3},
	{GL_FLOAT_MAT4, CompiledResult::Type::Mat4},
	{GL_FLOAT_MAT2x3, CompiledResult::Type::Mat2x3},
	{GL_FLOAT_MAT2x3, CompiledResult::Type::Mat2x4},
	{GL_FLOAT_MAT3x2, CompiledResult::Type::Mat3x2},
	{GL_FLOAT_MAT3x4, CompiledResult::Type::Mat3x4},
	{GL_FLOAT_MAT4x2, CompiledResult::Type::Mat4x2},
	{GL_FLOAT_MAT4x3, CompiledResult::Type::Mat4x3},
	{GL_DOUBLE_MAT2, CompiledResult::Type::DMat2},
	{GL_DOUBLE_MAT3, CompiledResult::Type::DMat3},
	{GL_DOUBLE_MAT4, CompiledResult::Type::DMat4},
	{GL_DOUBLE_MAT2x3, CompiledResult::Type::DMat2x3},
	{GL_DOUBLE_MAT2x4, CompiledResult::Type::DMat2x4},
	{GL_DOUBLE_MAT3x2, CompiledResult::Type::DMat3x2},
	{GL_DOUBLE_MAT3x4, CompiledResult::Type::DMat3x4},
	{GL_DOUBLE_MAT4x2, CompiledResult::Type::DMat4x2},
	{GL_DOUBLE_MAT4x3, CompiledResult::Type::DMat4x3},

	// Samplers
	{GL_SAMPLER_1D, CompiledResult::Type::Sampler1D},
	{GL_SAMPLER_2D, CompiledResult::Type::Sampler2D},
	{GL_SAMPLER_3D, CompiledResult::Type::Sampler3D},
	{GL_SAMPLER_CUBE, CompiledResult::Type::SamplerCube},
	{GL_SAMPLER_1D_SHADOW, CompiledResult::Type::Sampler1DShadow},
	{GL_SAMPLER_2D_SHADOW, CompiledResult::Type::Sampler2DShadow},
	{GL_SAMPLER_1D_ARRAY, CompiledResult::Type::Sampler1DArray},
	{GL_SAMPLER_2D_ARRAY, CompiledResult::Type::Sampler2DArray},
	{GL_SAMPLER_1D_ARRAY_SHADOW, CompiledResult::Type::Sampler1DArrayShadow},
	{GL_SAMPLER_2D_ARRAY_SHADOW, CompiledResult::Type::Sampler2DArrayShadow},
	{GL_SAMPLER_2D_MULTISAMPLE, CompiledResult::Type::Sampler2DMS},
	{GL_SAMPLER_2D_MULTISAMPLE_ARRAY, CompiledResult::Type::Sampler2DMSArray},
	{GL_SAMPLER_CUBE_SHADOW, CompiledResult::Type::SamplerCubeShadow},
	{GL_SAMPLER_BUFFER, CompiledResult::Type::SamplerBuffer},
	{GL_SAMPLER_2D_RECT, CompiledResult::Type::Sampler2DRect},
	{GL_SAMPLER_2D_RECT_SHADOW, CompiledResult::Type::Sampler2DRectShadow},
	{GL_INT_SAMPLER_1D, CompiledResult::Type::ISampler1D},
	{GL_INT_SAMPLER_2D, CompiledResult::Type::ISampler2D},
	{GL_INT_SAMPLER_3D, CompiledResult::Type::ISampler3D},
	{GL_INT_SAMPLER_CUBE, CompiledResult::Type::ISamplerCube},
	{GL_INT_SAMPLER_1D_ARRAY, CompiledResult::Type::ISampler1DArray},
	{GL_INT_SAMPLER_2D_ARRAY, CompiledResult::Type::ISampler2DArray},
	{GL_INT_SAMPLER_2D_MULTISAMPLE, CompiledResult::Type::ISampler2DMS},
	{GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, CompiledResult::Type::ISampler2DMSArray},
	{GL_INT_SAMPLER_2D_RECT, CompiledResult::Type::ISampler2DRect},
	{GL_UNSIGNED_INT_SAMPLER_1D, CompiledResult::Type::USampler1D},
	{GL_UNSIGNED_INT_SAMPLER_2D, CompiledResult::Type::USampler2D},
	{GL_UNSIGNED_INT_SAMPLER_3D, CompiledResult::Type::USampler3D},
	{GL_UNSIGNED_INT_SAMPLER_CUBE, CompiledResult::Type::USamplerCube},
	{GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, CompiledResult::Type::USampler1DArray},
	{GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, CompiledResult::Type::USampler2DArray},
	{GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, CompiledResult::Type::USampler2DMS},
	{GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, CompiledResult::Type::USampler2DMSArray},
	{GL_UNSIGNED_INT_SAMPLER_2D_RECT, CompiledResult::Type::USampler2DRect},
};

static std::string readLine(std::istream& stream)
{
	std::string line;
	int c;
	while (stream)
	{
		c = stream.get();
		if (c == std::char_traits<char>::eof() || c == '\n')
			break;

		line.push_back(static_cast<char>(c));
	}
	return line;
}

// Adapted from glslang validator. Don't use their version directly because we want to store the
// output in the Output class and strtok isn't thread-safe.
static bool decodeResourceLimits(Output& output, TBuiltInResource& resources, std::istream& stream,
	const std::string& fileName)
{
	std::vector<std::string> splitStrings;
	std::size_t lineNumber = 0;
    while (stream)
	{
		++lineNumber;
		std::string line = readLine(stream);
		boost::algorithm::trim(line);
		if (line.empty() || line[0] == '#')
			continue;

		splitStrings.clear();
		boost::algorithm::split(splitStrings, line, boost::algorithm::is_space(),
			boost::algorithm::token_compress_on);
		const char* token = nullptr;
		const char* valueStr = nullptr;
		if (splitStrings.size() >= 1)
			token = splitStrings[0].c_str();
		if (splitStrings.size() >= 2)
			valueStr = splitStrings[1].c_str();
        if (valueStr == 0 || ! (valueStr[0] == '-' || (valueStr[0] >= '0' && valueStr[0] <= '9')))
		{
			output.addMessage(Output::Level::Error, fileName, lineNumber, 0, false,
				"resource configuration syntax error: each name must be followed by one number");
            return false;
        }
        int value = atoi(valueStr);

        if (strcmp(token, "MaxLights") == 0)
            resources.maxLights = value;
        else if (strcmp(token, "MaxClipPlanes") == 0)
            resources.maxClipPlanes = value;
        else if (strcmp(token, "MaxTextureUnits") == 0)
            resources.maxTextureUnits = value;
        else if (strcmp(token, "MaxTextureCoords") == 0)
            resources.maxTextureCoords = value;
        else if (strcmp(token, "MaxVertexAttribs") == 0)
            resources.maxVertexAttribs = value;
        else if (strcmp(token, "MaxVertexUniformComponents") == 0)
            resources.maxVertexUniformComponents = value;
        else if (strcmp(token, "MaxVaryingFloats") == 0)
            resources.maxVaryingFloats = value;
        else if (strcmp(token, "MaxVertexTextureImageUnits") == 0)
            resources.maxVertexTextureImageUnits = value;
        else if (strcmp(token, "MaxCombinedTextureImageUnits") == 0)
            resources.maxCombinedTextureImageUnits = value;
        else if (strcmp(token, "MaxTextureImageUnits") == 0)
            resources.maxTextureImageUnits = value;
        else if (strcmp(token, "MaxFragmentUniformComponents") == 0)
            resources.maxFragmentUniformComponents = value;
        else if (strcmp(token, "MaxDrawBuffers") == 0)
            resources.maxDrawBuffers = value;
        else if (strcmp(token, "MaxVertexUniformVectors") == 0)
            resources.maxVertexUniformVectors = value;
        else if (strcmp(token, "MaxVaryingVectors") == 0)
            resources.maxVaryingVectors = value;
        else if (strcmp(token, "MaxFragmentUniformVectors") == 0)
            resources.maxFragmentUniformVectors = value;
        else if (strcmp(token, "MaxVertexOutputVectors") == 0)
            resources.maxVertexOutputVectors = value;
        else if (strcmp(token, "MaxFragmentInputVectors") == 0)
            resources.maxFragmentInputVectors = value;
        else if (strcmp(token, "MinProgramTexelOffset") == 0)
            resources.minProgramTexelOffset = value;
        else if (strcmp(token, "MaxProgramTexelOffset") == 0)
            resources.maxProgramTexelOffset = value;
        else if (strcmp(token, "MaxClipDistances") == 0)
            resources.maxClipDistances = value;
        else if (strcmp(token, "MaxComputeWorkGroupCountX") == 0)
            resources.maxComputeWorkGroupCountX = value;
        else if (strcmp(token, "MaxComputeWorkGroupCountY") == 0)
            resources.maxComputeWorkGroupCountY = value;
        else if (strcmp(token, "MaxComputeWorkGroupCountZ") == 0)
            resources.maxComputeWorkGroupCountZ = value;
        else if (strcmp(token, "MaxComputeWorkGroupSizeX") == 0)
            resources.maxComputeWorkGroupSizeX = value;
        else if (strcmp(token, "MaxComputeWorkGroupSizeY") == 0)
            resources.maxComputeWorkGroupSizeY = value;
        else if (strcmp(token, "MaxComputeWorkGroupSizeZ") == 0)
            resources.maxComputeWorkGroupSizeZ = value;
        else if (strcmp(token, "MaxComputeUniformComponents") == 0)
            resources.maxComputeUniformComponents = value;
        else if (strcmp(token, "MaxComputeTextureImageUnits") == 0)
            resources.maxComputeTextureImageUnits = value;
        else if (strcmp(token, "MaxComputeImageUniforms") == 0)
            resources.maxComputeImageUniforms = value;
        else if (strcmp(token, "MaxComputeAtomicCounters") == 0)
            resources.maxComputeAtomicCounters = value;
        else if (strcmp(token, "MaxComputeAtomicCounterBuffers") == 0)
            resources.maxComputeAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxVaryingComponents") == 0)
            resources.maxVaryingComponents = value;
        else if (strcmp(token, "MaxVertexOutputComponents") == 0)
            resources.maxVertexOutputComponents = value;
        else if (strcmp(token, "MaxGeometryInputComponents") == 0)
            resources.maxGeometryInputComponents = value;
        else if (strcmp(token, "MaxGeometryOutputComponents") == 0)
            resources.maxGeometryOutputComponents = value;
        else if (strcmp(token, "MaxFragmentInputComponents") == 0)
            resources.maxFragmentInputComponents = value;
        else if (strcmp(token, "MaxImageUnits") == 0)
            resources.maxImageUnits = value;
        else if (strcmp(token, "MaxCombinedImageUnitsAndFragmentOutputs") == 0)
            resources.maxCombinedImageUnitsAndFragmentOutputs = value;
        else if (strcmp(token, "MaxCombinedShaderOutputResources") == 0)
            resources.maxCombinedShaderOutputResources = value;
        else if (strcmp(token, "MaxImageSamples") == 0)
            resources.maxImageSamples = value;
        else if (strcmp(token, "MaxVertexImageUniforms") == 0)
            resources.maxVertexImageUniforms = value;
        else if (strcmp(token, "MaxTessControlImageUniforms") == 0)
            resources.maxTessControlImageUniforms = value;
        else if (strcmp(token, "MaxTessEvaluationImageUniforms") == 0)
            resources.maxTessEvaluationImageUniforms = value;
        else if (strcmp(token, "MaxGeometryImageUniforms") == 0)
            resources.maxGeometryImageUniforms = value;
        else if (strcmp(token, "MaxFragmentImageUniforms") == 0)
            resources.maxFragmentImageUniforms = value;
        else if (strcmp(token, "MaxCombinedImageUniforms") == 0)
            resources.maxCombinedImageUniforms = value;
        else if (strcmp(token, "MaxGeometryTextureImageUnits") == 0)
            resources.maxGeometryTextureImageUnits = value;
        else if (strcmp(token, "MaxGeometryOutputVertices") == 0)
            resources.maxGeometryOutputVertices = value;
        else if (strcmp(token, "MaxGeometryTotalOutputComponents") == 0)
            resources.maxGeometryTotalOutputComponents = value;
        else if (strcmp(token, "MaxGeometryUniformComponents") == 0)
            resources.maxGeometryUniformComponents = value;
        else if (strcmp(token, "MaxGeometryVaryingComponents") == 0)
            resources.maxGeometryVaryingComponents = value;
        else if (strcmp(token, "MaxTessControlInputComponents") == 0)
            resources.maxTessControlInputComponents = value;
        else if (strcmp(token, "MaxTessControlOutputComponents") == 0)
            resources.maxTessControlOutputComponents = value;
        else if (strcmp(token, "MaxTessControlTextureImageUnits") == 0)
            resources.maxTessControlTextureImageUnits = value;
        else if (strcmp(token, "MaxTessControlUniformComponents") == 0)
            resources.maxTessControlUniformComponents = value;
        else if (strcmp(token, "MaxTessControlTotalOutputComponents") == 0)
            resources.maxTessControlTotalOutputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationInputComponents") == 0)
            resources.maxTessEvaluationInputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationOutputComponents") == 0)
            resources.maxTessEvaluationOutputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationTextureImageUnits") == 0)
            resources.maxTessEvaluationTextureImageUnits = value;
        else if (strcmp(token, "MaxTessEvaluationUniformComponents") == 0)
            resources.maxTessEvaluationUniformComponents = value;
        else if (strcmp(token, "MaxTessPatchComponents") == 0)
            resources.maxTessPatchComponents = value;
        else if (strcmp(token, "MaxPatchVertices") == 0)
            resources.maxPatchVertices = value;
        else if (strcmp(token, "MaxTessGenLevel") == 0)
            resources.maxTessGenLevel = value;
        else if (strcmp(token, "MaxViewports") == 0)
            resources.maxViewports = value;
        else if (strcmp(token, "MaxVertexAtomicCounters") == 0)
            resources.maxVertexAtomicCounters = value;
        else if (strcmp(token, "MaxTessControlAtomicCounters") == 0)
            resources.maxTessControlAtomicCounters = value;
        else if (strcmp(token, "MaxTessEvaluationAtomicCounters") == 0)
            resources.maxTessEvaluationAtomicCounters = value;
        else if (strcmp(token, "MaxGeometryAtomicCounters") == 0)
            resources.maxGeometryAtomicCounters = value;
        else if (strcmp(token, "MaxFragmentAtomicCounters") == 0)
            resources.maxFragmentAtomicCounters = value;
        else if (strcmp(token, "MaxCombinedAtomicCounters") == 0)
            resources.maxCombinedAtomicCounters = value;
        else if (strcmp(token, "MaxAtomicCounterBindings") == 0)
            resources.maxAtomicCounterBindings = value;
        else if (strcmp(token, "MaxVertexAtomicCounterBuffers") == 0)
            resources.maxVertexAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxTessControlAtomicCounterBuffers") == 0)
            resources.maxTessControlAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxTessEvaluationAtomicCounterBuffers") == 0)
            resources.maxTessEvaluationAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxGeometryAtomicCounterBuffers") == 0)
            resources.maxGeometryAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxFragmentAtomicCounterBuffers") == 0)
            resources.maxFragmentAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxCombinedAtomicCounterBuffers") == 0)
            resources.maxCombinedAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxAtomicCounterBufferSize") == 0)
            resources.maxAtomicCounterBufferSize = value;
        else if (strcmp(token, "MaxTransformFeedbackBuffers") == 0)
            resources.maxTransformFeedbackBuffers = value;
        else if (strcmp(token, "MaxTransformFeedbackInterleavedComponents") == 0)
            resources.maxTransformFeedbackInterleavedComponents = value;
        else if (strcmp(token, "MaxCullDistances") == 0)
            resources.maxCullDistances = value;
        else if (strcmp(token, "MaxCombinedClipAndCullDistances") == 0)
            resources.maxCombinedClipAndCullDistances = value;
        else if (strcmp(token, "MaxSamples") == 0)
            resources.maxSamples = value;

        else if (strcmp(token, "nonInductiveForLoops") == 0)
            resources.limits.nonInductiveForLoops = (value != 0);
        else if (strcmp(token, "whileLoops") == 0)
            resources.limits.whileLoops = (value != 0);
        else if (strcmp(token, "doWhileLoops") == 0)
            resources.limits.doWhileLoops = (value != 0);
        else if (strcmp(token, "generalUniformIndexing") == 0)
            resources.limits.generalUniformIndexing = (value != 0);
        else if (strcmp(token, "generalAttributeMatrixVectorIndexing") == 0)
            resources.limits.generalAttributeMatrixVectorIndexing = (value != 0);
        else if (strcmp(token, "generalVaryingIndexing") == 0)
            resources.limits.generalVaryingIndexing = (value != 0);
        else if (strcmp(token, "generalSamplerIndexing") == 0)
            resources.limits.generalSamplerIndexing = (value != 0);
        else if (strcmp(token, "generalVariableIndexing") == 0)
            resources.limits.generalVariableIndexing = (value != 0);
        else if (strcmp(token, "generalConstantMatrixVectorIndexing") == 0)
            resources.limits.generalConstantMatrixVectorIndexing = (value != 0);
        else
		{
			output.addMessage(Output::Level::Warning, fileName, lineNumber, 0, false,
				"unrecognized resource type: " + splitStrings[0]);
		}
    }

	return true;
}

const Target::FeatureInfo& Target::getFeatureInfo(Target::Feature feature)
{
	return featureInfos[static_cast<unsigned int>(feature)];
}

Target::Target()
	: m_remapVariables(false)
	, m_optimize(false)
	, m_stripDebug(false)
{
	Compiler::initialize();
	m_featureStates.fill(State::Default);
}

Target::~Target()
{
	Compiler::shutdown();
}

bool Target::isFeatureOverridden(Feature feature) const
{
	return m_featureStates[static_cast<unsigned int>(feature)] != State::Default;
}

void Target::overrideFeature(Feature feature, bool enabled)
{
	m_featureStates[static_cast<unsigned int>(feature)] =
		enabled ? State::Enabled : State::Disabled;
}

void Target::clearOverride(Feature feature)
{
	m_featureStates[static_cast<unsigned int>(feature)] = State::Default;
}

bool Target::featureEnabled(Feature feature) const
{
	State state = m_featureStates[static_cast<unsigned int>(feature)];
	if (state == State::Default)
		return featureSupported(feature);

	return state == State::Enabled;
}

void Target::addIncludePath(std::string path)
{
	m_includePaths.push_back(std::move(path));
}

const std::vector<std::string>& Target::getIncludePaths() const
{
	return m_includePaths;
}

void Target::clearIncludePaths()
{
	m_includePaths.clear();
}

void Target::addDefine(std::string name, std::string value)
{
	m_defines.emplace_back(std::move(name), std::move(value));
}

const std::vector<std::pair<std::string, std::string>>& Target::getDefines() const
{
	return m_defines;
}

std::vector<std::pair<std::string, std::string>> Target::getExtraDefines() const
{
	return std::vector<std::pair<std::string, std::string>>();
}

void Target::clearDefines()
{
	m_defines.clear();
}

const std::string& Target::getSpirVToolCommand() const
{
	return m_spirVToolCommand;
}

void Target::setSpirVToolCommand(std::string command)
{
	m_spirVToolCommand = std::move(command);
}

bool Target::getRemapVariables() const
{
	return m_remapVariables;
}

void Target::setRemapVariables(bool remap)
{
	m_remapVariables = remap;
}

bool Target::getOptimize() const
{
	return m_optimize;
}

void Target::setOptimize(bool optimize)
{
	m_optimize = optimize;
}

bool Target::getStripDebug() const
{
	return m_stripDebug;
}

void Target::setStripDebug(bool strip)
{
	m_stripDebug = strip;
}

const std::string& Target::getResourcesFileName() const
{
	return m_resourcesFile;
}

void Target::setResourcesFileName(std::string fileName)
{
	m_resourcesFile = std::move(fileName);
}

bool Target::compile(CompiledResult& result, Output& output, const std::string& fileName)
{
	Preprocessor preprocessor;
	setupPreprocessor(preprocessor);

	Parser parser;
	if (!preprocessor.preprocess(parser.getTokens(), output, fileName))
		return false;

	return compileImpl(result, output, parser, fileName);
}

bool Target::compile(CompiledResult& result, Output& output, std::istream& stream,
	const std::string& fileName)
{
	Preprocessor preprocessor;
	setupPreprocessor(preprocessor);

	Parser parser;
	if (!preprocessor.preprocess(parser.getTokens(), output, stream, fileName))
		return false;

	return compileImpl(result, output, parser, fileName);
}

bool Target::finish(CompiledResult& result, Output& output)
{
	if (result.m_target != this)
	{
		output.addMessage(Output::Level::Error, "<finish>", 0, 0, false,
			"internal error: targets don't match in compiled result");
		return false;
	}

	result.m_sharedData.clear();
	if (!getSharedData(result.m_sharedData, output))
		return false;

	return true;
}

bool Target::getSharedData(std::vector<std::uint8_t>&, Output&)
{
	return true;
}

void Target::setupPreprocessor(Preprocessor& preprocessor) const
{
	for (const std::string& include : m_includePaths)
		preprocessor.addIncludePath(include);

	for (const auto& define : m_defines)
		preprocessor.addDefine(define.first, define.second);

	for (auto& define : getExtraDefines())
		preprocessor.addDefine(std::move(define.first), std::move(define.second));

	for (unsigned int i = 0; i < featureCount; ++i)
	{
		if (featureEnabled(static_cast<Feature>(i)))
			preprocessor.addDefine(featureInfos[i].define, "1");
		else
			preprocessor.addDefine(featureInfos[i].define, "0");
	}
}

bool Target::compileImpl(CompiledResult& result, Output& output, Parser& parser,
	const std::string& fileName)
{
	int options = 0;
	if (!featureEnabled(Feature::UniformBuffers))
		options |= Parser::RemoveUniformBlocks;

	if (!parser.parse(output, options))
		return false;

	// Set the target info on the result.
	if (!result.m_target)
	{
		result.m_target = this;
		result.m_targetId = getId();
		result.m_targetVersion = getVersion();
	}
	else if (result.m_target != this)
	{
		output.addMessage(Output::Level::Error, fileName, 0, 0, false,
			"internal error: targets don't match in compiled result");
		return false;
	}

	// Read in the resource limits.
	TBuiltInResource resources = glslang::DefaultTBuiltInResource;
	if (!m_resourcesFile.empty())
	{
		std::ifstream stream(m_resourcesFile);
		if (stream.is_open())
		{
			if (!decodeResourceLimits(output, resources, stream, m_resourcesFile))
				return false;
		}
		else
		{
			output.addMessage(Output::Level::Error, fileName, 0, 0, false,
				"cannot read resources file: " + m_resourcesFile);
			return false;
		}
	}

	// Compile the pipelines.
	int processOptions = 0;
	if (m_remapVariables)
		processOptions |= Compiler::RemapVariables;
	if (m_optimize)
		processOptions |= Compiler::Optimize;
	if (m_stripDebug)
		processOptions |= Compiler::StripDebug;

	std::vector<char> tempData;
	std::vector<uint8_t> shaderData;
	std::vector<Parser::LineMapping> lineMappings;
	for (const Parser::Pipeline& pipeline : parser.getPipelines())
	{
		// Add the current pipeline to the result.
		auto addPair = result.m_pipelines.emplace(pipeline.name,
			CompiledResult::Pipeline());
		if (!addPair.second)
		{
			output.addMessage(Output::Level::Error, pipeline.token->fileName, pipeline.token->line,
				pipeline.token->column, false, "pipeline already declared: " + pipeline.name);
			output.addMessage(Output::Level::Error, addPair.first->second.file,
				addPair.first->second.line, addPair.first->second.column, true,
				"see previous declaration");
			return false;
		}

		CompiledResult::Pipeline& addedPipeline = addPair.first->second;
		addedPipeline.file = pipeline.token->fileName;
		addedPipeline.line = pipeline.token->line;
		addedPipeline.column = pipeline.token->column;

		// Compile the stages.
		Compiler::Stages stages;
		for (unsigned int i = 0; i < Parser::stageCount; ++i)
		{
			auto stage = static_cast<Parser::Stage>(i);
			if (pipeline.entryPoints[i].empty())
				continue;

			std::string glsl = parser.createShaderString(lineMappings, pipeline, stage);
			if (!Compiler::compile(stages, output, fileName, glsl, lineMappings, stage, resources))
				return false;
		}

		// Link the program.
		glslang::TProgram program;
		if (!Compiler::link(program, output, pipeline, stages))
			return false;
		// Add the reflection info.
		if (!program.buildReflection())
		{
			output.addMessage(Output::Level::Error, fileName, 0, 0, false,
				"internal error: failed to build reflection");
			return false;
		}

		// Cross-compile the stages.
		for (unsigned int i = 0; i < Parser::stageCount; ++i)
		{
			auto stage = static_cast<Parser::Stage>(i);

			if (!stages.shaders[i])
				continue;

			// Create SPIR-V.
			Compiler::SpirV spirV = Compiler::assemble(output, program, stage, pipeline);
			if (spirV.empty())
				return false;
			Compiler::process(spirV, processOptions);

			// Use external command if set.
			if (!m_spirVToolCommand.empty())
			{
				ExecuteCommand command;
				command.getInput().write(reinterpret_cast<const char*>(spirV.data()),
					spirV.size()*sizeof(std::uint32_t));
				if (!command.execute(output, m_spirVToolCommand))
					return false;

				tempData.assign(std::istreambuf_iterator<char>(command.getOutput().rdbuf()),
					std::istreambuf_iterator<char>());
				if ((tempData.size() % sizeof(std::uint32_t)) != 0)
				{
					output.addMessage(Output::Level::Error, fileName, 0, 0, false,
						"command output invalid spir-v: " + m_spirVToolCommand);
					return false;
				}

				spirV.reserve(tempData.size()/sizeof(std::uint32_t));
				std::memcpy(spirV.data(), tempData.data(), tempData.size());
			}

			shaderData.clear();
			if (!crossCompile(shaderData, output, static_cast<Stage>(stage), spirV,
				pipeline.entryPoints[i], fileName, pipeline.token->line, pipeline.token->column))
			{
				return false;
			}

			std::size_t shaderIndex = result.addShader(std::move(shaderData));
			switch (stage)
			{
				case Parser::Stage::Vertex:
					addedPipeline.vertex = shaderIndex;
					break;
				case Parser::Stage::TessellationControl:
					addedPipeline.tessellationControl = shaderIndex;
					break;
				case Parser::Stage::TessellationEvaluation:
					addedPipeline.tessellationEvaluation = shaderIndex;
					break;
				case Parser::Stage::Geometry:
					addedPipeline.geometry = shaderIndex;
					break;
				case Parser::Stage::Fragment:
					addedPipeline.fragment = shaderIndex;
					break;
				case Parser::Stage::Compute:
					addedPipeline.compute = shaderIndex;
					break;
			}
		}

		// Add reflection info.
		// Uniforms
		addedPipeline.uniforms.resize(program.getNumLiveUniformVariables());
		for (int i = 0; i < program.getNumLiveUniformVariables(); ++i)
		{
			addedPipeline.uniforms[i].name = program.getUniformName(i);

			auto foundType = typeMap.find(program.getUniformType(i));
			if (foundType == typeMap.end())
			{
				std::stringstream stream;
				stream << "internal error: unknown OpenGL type for uniform variable " <<
					addedPipeline.uniforms[i].name << ": " << std::hex << program.getUniformType(i);
				output.addMessage(Output::Level::Error, fileName, 0, 0, false, stream.str());
				return false;
			}
			addedPipeline.uniforms[i].type = foundType->second;

			addedPipeline.uniforms[i].blockIndex = program.getUniformBlockIndex(i);
			addedPipeline.uniforms[i].bufferOffset = program.getUniformBufferOffset(i);
			addedPipeline.uniforms[i].elements = program.getUniformArraySize(i);
		}

		// Uniform blocks
		addedPipeline.uniformBlocks.resize(program.getNumLiveUniformBlocks());
		for (int i = 0; i < program.getNumLiveUniformBlocks(); ++i)
		{
			addedPipeline.uniformBlocks[i].name = program.getUniformBlockName(i);
			addedPipeline.uniformBlocks[i].size = program.getUniformBlockSize(i);
		}

		// Attributes
		addedPipeline.attributes.resize(program.getNumLiveAttributes());
		for (int i = 0; i < program.getNumLiveAttributes(); ++i)
		{
			addedPipeline.attributes[i].name = program.getAttributeName(i);

			auto foundType = typeMap.find(program.getAttributeType(i));
			if (foundType == typeMap.end())
			{
				std::stringstream stream;
				stream << "internal error: unknown OpenGL type for vertex attribute " <<
					addedPipeline.attributes[i].name << ": " << std::hex <<
					program.getUniformType(i);
				output.addMessage(Output::Level::Error, fileName, 0, 0, false, stream.str());
				return false;
			}
			addedPipeline.attributes[i].type = foundType->second;
		}
	}

	return true;
}

} // namespace
