/*
 * Copyright 2016-2025 Aaron Barany
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

#include <MSL/Config.h>
#include <MSL/Compile/Export.h>
#include <MSL/Compile/Types.h>
#include <array>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

/**
 * @file
 * @brief Base class for a target.
 */

namespace msl
{

class CompiledResult;
class Output;
class Parser;
class Preprocessor;

/**
 * @brief Base class for a target.
 *
 * This contains the capabilities of the target and extra info for how to compile the shader
 * to the final form.
 *
 * When subclassing, the following virtual functions should be overloaded:
 * - getId(): gets the ID specific to the target language. The MSL_CREATE_ID() function should be
 *   used to generate the ID.
 * - getVersion(): gets the version of the target language.
 * - featureSupported(): determine whether or not a feature is supported by the target.
 * - getExtraDefines(): gets target-specific defines to automatically add during preprocessing.
 * - crossCompile(): convert SPIR-V to the target language.
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
		TextureArrays,        ///< Texture arrays.
		ShadowSamplers,       ///< Shadow texture samplers.
		MultisampledTextures, ///< Multisampled texture samplers.
		IntegerTextures,      ///< Integer texture samplers.
		Images,               ///< Image types.

		// Storage
		UniformBlocks,  ///< Uniform blocks. If disabled, uniform blocks will be in the push
		                ///< constant section. (equivalent to individual uniforms)
		Buffers,        ///< Generic buffer types.
		Std140,         ///< std140 block layout.
		Std430,         ///< std430 block layout.
		BindingPoints,  ///< Allows setting the binding index in the layout.
		DescriptorSets, ///< Allows setting the descriptor set index in the layout.

		// Pipeline stages
		TessellationStages, ///< Tessellation control and evaluation stages.
		GeometryStage,      ///< Geometry stage.
		ComputeStage,       ///< Compute stage.

		// Fragment shader outputs
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
		SubpassInputs,          ///< Subpass inputs for reading directly from framebuffers.
		ClipDistance,           ///< Support for gl_ClipDistance array.
		CullDistance,           ///< Support for gl_CullDistance array.
		EarlyFragmentTests,     ///< Support for explicitly enabling early fragment tests.
		FragmentInputs,         ///< Support for reading results of other fragment shaders.
	};

	/**
	 * @brief The number of feature enum elements.
	 */
	static const unsigned int featureCount =
		static_cast<unsigned int>(Feature::FragmentInputs) + 1;

	/**
	 * @brief Enum for the optimization passes to run.
	 */
	enum class Optimize
	{
		None,    ///< Don't perform any optimizations.
		Minimal, ///< Minimal optimizations such as dead-code removal.
		Full     ///< Full optimization passes.
	};

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
	 * @brief Gets the ID for the target.
	 *
	 * This is generally created with the MSL_CREATE_ID() macro to combine 4 characters into
	 * an ID.
	 *
	 * @return The target ID.
	 */
	virtual std::uint32_t getId() const = 0;

	/**
	 * @brief Gets the version for the target.
	 * @return The target version.
	 */
	virtual std::uint32_t getVersion() const = 0;

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
	 * @brief Gets the defines set by the user with addDefine().
	 * @return The defines. For each pair, the first element is the name, while the second is the
	 * value.
	 */
	const std::vector<std::pair<std::string, std::string>>& getDefines() const;

	/**
	 * @brief Gets extra defines for the target.
	 * @return The extra defines. For each pair, the first element is the name and the second
	 * element is the value.
	 */
	virtual std::vector<std::pair<std::string, std::string>> getExtraDefines() const;

	/**
	 * @brief Clears the defines.
	 */
	void clearDefines();

	/**
	 * @brief Adds a header line to to be added to the source before any processing.
	 * @param header The header line.
	 */
	void addPreHeaderLine(std::string header);

	/**
	 * @brief Gets the header lines to be added before any processing.
	 * @return The header lines.
	 */
	const std::vector<std::string>& getPreHeaderLines() const;

	/**
	 * @brief Clears the pre-header lines.
	 */
	void clearPreHeaderLines();

	/**
	 * @brief Gets the tool command to run on the output SPIR-V before cross-compiling.
	 * @return The SPIR-V tool command.
	 */
	const std::string& getSpirVToolCommand() const;

	/**
	 * @brief Sets the SPIR-V tool command to run on the output SPIR-V before cross-compiling.
	 *
	 * This can be used to hook external tools, such as optimizers, into the shader compilation
	 * pipeline. Compilation will fail if the tool returns a non-zero error code. Output from the
	 * tool will be captured and added to the Output instance.
	 *
	 * The string $input will be replaced with the file name for the input file, while the string
	 * $output will be replaced with the file name for the output file.
	 *
	 * When empty, no command will be run.
	 *
	 * @param command The command to run.
	 */
	void setSpirVToolCommand(std::string command);

	/**
	 * @brief Returns whether or not to remap the SPIR-V variables.
	 *
	 * This can be useful to improve compression ratios for SPIR-V targets.
	 *
	 * @return True to remap variables.
	 */
	bool getRemapVariables() const;

	/**
	 * @brief Sets whether or not to remap the SPIR-V variables.
	 *
	 * This can be useful to improve compression ratios for SPIR-V targets.
	 *
	 * @param remap True to remap variables.
	 */
	void setRemapVariables(bool remap);

	/**
	 * @brief Retruns the optimization mode.
	 * @return The optimization mode.
	 */
	Optimize getOptimize() const;

	/**
	 * @brief Sets the optimization mode.
	 * @param optimize The optimization mode.
	 */
	void setOptimize(Optimize optimize);

	/**
	 * @brief Returns whether or not to strip the debug symbols from SPIR-V.
	 * @return True to strip debug.
	 */
	bool getStripDebug() const;

	/**
	 * @brief Sets whether or not to strip the debug symbols from SPIR-V.
	 * @param strip True to strip debug.
	 */
	void setStripDebug(bool strip);

	/**
	 * @brief Gets whether or not to add dummy descriptor sets and bindings.
	 * @return True to add dummy bindings.
	 */
	bool getDummyBindings() const;

	/**
	 * @brief Sets whether or not to add dummy descriptor sets and bindings.
	 *
	 * This can be done for SPIR-V to assign the bndings at runtime before sending them to Vulkan.
	 * Unlike adjustable bindings, it is assumed a local copy of SPIR-V will be created to change
	 * the bindings at runtime.
	 *
	 * @param dummy True to add dummy bindings.
	 */
	void setDummyBindings(bool dummy);

	/**
	 * @brief Gets whether or not to use adjustable descriptor sets and bindings.
	 * @return True to use adjustable bindings.
	 */
	bool getAdjustableBindings() const;

	/**
	 * @brief Sets whether or not to use adjustable descriptor sets and bindings.
	 *
	 * This can be done for SPIR-V to assign the bndings at runtime before sending them to Vulkan.
	 * This will modify the loaded module in place. No duplicate shader results will be removed to
	 * ensure that each one can have the bindings set separately.
	 *
	 * @param adjustable True to use adjustable bindings.
	 */
	void setAdjustableBindings(bool adjustable);

	/**
	 * @brief Gets the file name to a text file describing the resource limits.
	 *
	 * This is the same format as used by the glslang validator tool. When empty, the default
	 * resource limits are used.
	 *
	 * @return The reosurce file name.
	 */
	const std::string& getResourcesFileName() const;

	/**
	 * @brief Sets the file name to a text file describing the resource limits.
	 *
	 * This is the same format as used by the glslang validator tool. When empty, the default
	 * resource limits are used.
	 *
	 * @param fileName The reosurce file name.
	 */
	void setResourcesFileName(std::string fileName);

	/**
	 * @brief Compiles a shader.
	 * @param result The compiled result.
	 * @param output The output for warnings and errors.
	 * @param fileName The name of the file to load.
	 * @return False if compilation failed.
	 */
	bool compile(CompiledResult& result, Output& output, const std::string& fileName);

	/**
	 * @brief Compiles a shader.
	 * @param result The compiled result.
	 * @param output The output for warnings and errors.
	 * @param stream The stream to read from.
	 * @param fileName The name of the file corresponding to the stream. This is used for error
	 * outputs.
	 * @return False if compilation failed.
	 */
	bool compile(CompiledResult& result, Output& output, std::istream& stream,
		const std::string& fileName);

	/**
	 * @brief Finishes compiling the shader.
	 * @param result The compiled result.
	 * @param output The output for warnings and errors.
	 * @return False if compilation failed.
	 */
	bool finish(CompiledResult& result, Output& output);

protected:

	/**
	 * @brief Gets whether or not reflection names should be kept when stripping debug info.
	 *
	 * Reflection names include the names for uniforms, inputs, and outputs.
	 *
	 * @return True if reflection names should be kept. Default implementation returns true.
	 */
	virtual bool needsReflectionNames() const;

	/**
	 * @brief Gets the SPIR-V version to compile with.
	 * @return The SPIR-V version. Defaults to the latest supported version.
	 */
	virtual std::uint32_t getSpirVVersion() const;

	/**
	 * @brief Function called when about to compile a shader.
	 */
	virtual void willCompile();

	/**
	 * @brief Cross-compiles SPIR-V to the final target.
	 *
	 * If an error occurred, a message should be added to output explaining why.
	 *
	 * @param[out] data The data from cross-compiling.
	 * @param output The output to add errors and warnings.
	 * @param fileName The file name for the message of any output message.
	 * @param line The line number for the message of any output message.
	 * @param column The column number for the message of any output message.
	 * @param pipelineStages The stages that are available in the full pipeline.
	 * @param stage The stage being compiled.
	 * @param spirv The SPIR-V input.
	 * @param entryPoint The name of the entry point. This can be used to rename main back to the
	 *     original entry point name.
	 * @param uniforms The uniforms used by the shader.
	 * @param uniformIds The SPIR-V IDs for each uniform in the uniforms array. This may be modified
	 *     if a separate meaning is needed by the target.
	 * @param fragmentInputs The fragment inputs used by the shader.
	 * @param fragmentGroup The fragment group for the pipeline. Set to unknown if not specified.
	 * @return False if the compilation failed.
	 */
	virtual bool crossCompile(std::vector<std::uint8_t>& data, Output& output,
		const std::string& fileName, std::size_t line, std::size_t column,
		const std::array<bool, compile::stageCount>& pipelineStages, compile::Stage stage,
		const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
		const std::vector<compile::Uniform>& uniforms, std::vector<std::uint32_t>& uniformIds,
		const std::vector<compile::FragmentInputGroup>& fragmentInputs,
		std::uint32_t fragmentGroup) = 0;

	/**
	 * @brief Gets the shared data for the compiled shader.
	 * @param data The shared data.
	 * @param output The output to add errors and warnings.
	 * @return False if the compilation failed.
	 */
	virtual bool getSharedData(std::vector<std::uint8_t>& data, Output& output);

private:
	enum class State
	{
		Default,
		Enabled,
		Disabled
	};

	void setupPreprocessor(Preprocessor& preprocessor) const;
	bool compileImpl(CompiledResult& result, Output& output, Parser& parser,
		const std::string& fileName);

	std::array<State, featureCount> m_featureStates;
	std::vector<std::string> m_includePaths;
	std::vector<std::pair<std::string, std::string>> m_defines;
	std::vector<std::string> m_preHeaderLines;
	std::string m_spirVToolCommand;

	bool m_remapVariables;
	bool m_stripDebug;
	bool m_dummyBindings;
	bool m_adjustableBindings;
	Optimize m_optimize;
	std::string m_resourcesFile;
};

} // namespace msl
