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

#pragma once

#include <MSL/Compile/Config.h>
#include <MSL/Compile/Export.h>
#include <array>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

namespace msl
{

class CompiledResult;
class Output;

/**
 * @brief Base class for a target.
 *
 * This contains the capabilities of the target and extra info for how to compile the shader
 * to the final form.
 */
class MSL_COMPILE_EXPORT Target
{
public:
	/**
	 * @brief List of features to query if they are supported.
	 */
	enum Feature
	{
		// Types
		Integers,             ///< Integer types.
		Doubles,              ///< Double types.
		NonSquareMatrices,    ///< Non-square matrices, such as Mat3x4.
		Texture3D,            ///< 3D textures.
		TextureArray,         ///< Texture arrays.
		ShadowSamplers,       ///< Shadow texture samplers.
		MultisampledTextures, ///< Multisampled texture samplers.
		IntegerTextures,      ///< Integer texture samplers.
		Images,               ///< Image types.

		// Storage
		UniformBuffers, ///< Uniform buffers. If disabled, uniform buffers will be converted
		                ///< to push constants. (equivalent to individual uniforms)
		Buffers,        ///< Generic buffer types.
		Std140,         ///< std140 block layout.
		Std430,         ///< std430 block layout.

		// Pipeline stages
		TessellationStages, ///< Tessellation control and evaluation stages.
		GeometryStage,      ///< Geometry stage.
		ComputeStage,       ///< Compute stage.

		// Fragment shader controls
		MultipleRenderTargets, ///< Supports writing to more than one render target.
		DualSourceBlending,    ///< Supports outputting two colors to the same output.
		DepthHints,            ///< Allow hints to be givin about the depth output.

		// Other functionality
		Derivatives,            ///< dFdx() an dFdy() functions.
		AdvancedDerivatives,    ///< Coarse and fine versions of dFdx() and dFdy().
		MemoryBarriers,         ///< Memory barrier functions.
		PrimitiveStreams,       ///< Primitive streams for geometry shaders.
		InterpolationFunctions, ///< Interpolation functions for centroid, sample, and offset.
		TextureGather,          ///< textureGather() functions.
		TexelFetch,             ///< texelFetch() functions.
		TextureSize,            ///< textureSize() functions.
		TextureQueryLod,        ///< texureQueryLod() functions.
		TextureQueryLevels,     ///< textureQueryLevels() functions.
		TextureSamples,         ///< textureSamples() functions.
		BitFunctions,           ///< Integer functions such as bitfieldInsert() and findMSB().
		PackingFunctions,       ///< Packing and unpacking functions such as packUnorm2x16().
	};

	/**
	 * @brief The number of feature enum elements.
	 */
	static const unsigned int featureCount =
		static_cast<unsigned int>(Feature::PackingFunctions) + 1;

	/**
	 * @brief Information about a feature.
	 *
	 * This can be used for configuration files and generation of help documentation.
	 */
	struct FeatureInfo
	{
		/**
		 * @brief String name for the feature.
		 */
		const char* name;

		/**
		 * @brief The name for the define.
		 */
		const char* define;

		/**
		 * @brief Help string for the feature.
		 */
		const char* help;
	};

	/**
	 * @brief Gets information about a feature.
	 * @param feature The feature to get the info for.
	 * @return
	 */
	static const FeatureInfo& getFeatureInfo(Feature feature);

	Target();
	virtual ~Target();

	/**
	 * @brief Returns whether or not a feature is overridden.
	 * @param feature The feature.
	 * @return True if the feature is overridden.
	 */
	bool isFeatureOverridden(Feature feature) const;

	/**
	 * @brief Overrides a feature.
	 *
	 * This can be done to disable features that are problematic on certain platforms or
	 * force a feature to be enabled when an extension is available.
	 *
	 * @param feature The feature to override.
	 * @param enabled Whether to enable or disable the feature.
	 */
	void overrideFeature(Feature feature, bool enabled);

	/**
	 * @brief Clears a feature override to have it use the default.
	 * @param feature The feature to clear the override.
	 */
	void clearOverride(Feature feature);

	/**
	 * @brief Returns whether or not a feature is eanbled.
	 * @param feature The feature to check.
	 * @return True if the feature is enabled.
	 */
	bool featureEnabled(Feature feature) const;

	/**
	 * @brief Returns whether or not a feature is supported.
	 *
	 * Subclasses must override this function to determine what features are supported.
	 *
	 * @param feature The feature to check.
	 * @return True if the feature is supported.
	 */
	virtual bool featureSupported(Feature feature) const = 0;

	/**
	 * @brief Adds an include path.
	 * @param path The path to add.
	 */
	void addIncludePath(std::string path);

	/**
	 * @brief Gets the include paths.
	 * @return The include paths.
	 */
	const std::vector<std::string>& getIncludePaths() const;

	/**
	 * @brief Clears the include paths.
	 */
	void clearIncludePaths();

	/**
	 * @brief Adds a define.
	 * @param name The name of the define.
	 * @param value The value of the define. This may be the empty string.
	 */
	void addDefine(std::string name, std::string value);

	/**
	 * @brief Gets the defines.
	 * @return The defines. The first element is the name, while the second is the value.
	 */
	const std::vector<std::pair<std::string, std::string>>& getDefines() const;

	/**
	 * @brief Clears the defines.
	 */
	void clearDefines();

	/**
	 * @brief Compiles a shader.
	 * @param result The compiled result.
	 * @param output The output for warnings and errors.
	 * @param fileName The name of the file to load.
	 */
	bool compile(CompiledResult& result, Output& output, const std::string& fileName);

	/**
	 * @brief Compiles a shader.
	 * @param result The compiled result.
	 * @param output The output for warnings and errors.
	 * @param stream The stream to read from.
	 * @param fileName The name of the file corresponding to the stream.
	 */
	bool compile(CompiledResult& result, Output& output, std::istream& stream,
		const std::string& fileName);

	/**
	 * @brief Gets the tool command to run on the output SPIR-V before cross-compiling.
	 * @return The SPIR-V tool command.
	 */
	const std::string& getSpirVToolCommand() const;

	/**
	 * @brief Sets the SPIR-V tool command to run on the output SPIR-V before cross-compiling.
	 *
	 * The string $input will be replaced with the file name for the input file, while the string
	 * $output will be replaced with the file name for the output file.
	 *
	 * When empty, no command will be run.
	 *
	 * @param command The command to run.
	 */
	void setSpirVToolCommand(std::string command);

protected:
	/**
	 * @brief Add language-specific defines.
	 * @param defines The list of defines to add to.
	 */
	virtual void addDefines(std::vector<std::pair<std::string, std::string>>& defines);

	/**
	 * @brief Cross-compiles SPIR-V to the final target.
	 * @param spirv The SPIR-V input.
	 * @return The cross-compiled result.
	 */
	virtual std::vector<std::uint8_t> crossCompile(const std::vector<std::uint32_t>& spirv) = 0;

private:
	enum class State
	{
		Default,
		Enabled,
		Disabled
	};

	std::array<State, featureCount> m_featureStates;
	std::vector<std::string> m_includePaths;
	std::vector<std::pair<std::string, std::string>> m_defines;
	std::string m_spirVToolCommand;
};

} // namespace msl
