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
#include <glslang/MachineIndependent/gl_types.h>
#include <StandAlone/ResourceLimits.h>
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

bool Target::compile(CompiledResult& result, Output& output, const std::string& fileName) const
{
	Preprocessor preprocessor;
	setupPreprocessor(preprocessor);

	Parser parser;
	if (!preprocessor.preprocess(parser.getTokens(), output, fileName))
		return false;

	return compileImpl(result, output, parser, fileName);
}

bool Target::compile(CompiledResult& result, Output& output, std::istream& stream,
	const std::string& fileName) const
{
	Preprocessor preprocessor;
	setupPreprocessor(preprocessor);

	Parser parser;
	if (!preprocessor.preprocess(parser.getTokens(), output, stream, fileName))
		return false;

	return compileImpl(result, output, parser, fileName);
}

std::vector<std::pair<std::string, std::string>> Target::getExtraDefines() const
{
	return std::vector<std::pair<std::string, std::string>>();
}

void Target::setupPreprocessor(Preprocessor& preprocessor) const
{
	for (const std::string& include : m_includePaths)
		preprocessor.addIncludePath(include);

	for (const auto& define : m_defines)
		preprocessor.addDefine(define.first, define.second);

	for (auto& define : getExtraDefines())
		preprocessor.addDefine(std::move(define.first), std::move(define.second));
}

bool Target::compileImpl(CompiledResult& result, Output& output, Parser& parser,
	const std::string& fileName) const
{
	int options = 0;
	if (!featureEnabled(Feature::UniformBuffers))
		options |= Parser::RemoveUniformBlocks;

	if (!parser.parse(output, options))
		return false;

	// Set the target info on the result.
	if (!result.m_targetSet)
	{
		result.m_targetId = getId();
		result.m_targetVersion = getVersion();
		result.m_targetSet = true;
	}
	else if (result.m_targetId != getId() || result.m_targetVersion != getVersion())
	{
		output.addMessage(Output::Level::Error, fileName, 0, 0, false,
			"internal error: targets don't match in compiled result");
		return false;
	}

	// Read in the resource limits.
	std::vector<char> tempData;
	TBuiltInResource resources = glslang::DefaultTBuiltInResource;
	if (!m_resourcesFile.empty())
	{
		std::ifstream stream(m_resourcesFile);
		if (stream.is_open())
		{
			tempData.assign(std::istreambuf_iterator<char>(stream.rdbuf()),
				std::istreambuf_iterator<char>());
			tempData.push_back(0);
			glslang::DecodeResourceLimits(&resources, tempData.data());
		}
		else
		{
			output.addMessage(Output::Level::Warning, fileName, 0, 0, false,
				"cannot read resources file: " + m_resourcesFile);
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

			std::vector<uint8_t> shaderData = crossCompile(output, spirV, fileName,
				pipeline.token->line, pipeline.token->column);
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
				stream << "internal error: unknown OpenGL type: " << std::hex <<
					program.getUniformType(i);
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

			auto foundType = typeMap.find(program.getUniformType(i));
			if (foundType == typeMap.end())
			{
				std::stringstream stream;
				stream << "internal error: unknown OpenGL type: " << std::hex <<
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
