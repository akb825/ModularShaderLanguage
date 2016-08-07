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

const Target::FeatureInfo& Target::getFeatureInfo(Target::Feature feature)
{
	return featureInfos[static_cast<unsigned int>(feature)];
}

Target::Target()
{
	m_featureStates.fill(State::Default);
}

Target::~Target()
{
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

void Target::addDefines(std::vector<std::pair<std::string, std::string>>&)
{
}

} // namespace
