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

#include <MSL/Config.h>
#include <MSL/Compile/Export.h>
#include <MSL/Compile/Target.h>
#include <array>
#include <vector>

/**
 * @file
 * @brief Target implementation for compiling to GLSL.
 */

namespace msl
{

/**
 * @brief Target implementation for compiling to GLSL, both for desktop OpenGL and OpenGL ES.
 *
 * When compiling for desktop GLSL, the following will be set:
 * - ID: MSL_CREATE_ID('G', 'L', 'S', 'L')
 * - Version: the GLSL version (e.g. 120, 450)
 * - GLSL_VERSION is defined to the version number when preprocesssing.
 *
 * When compiling for GLSL ES, the following will be set:
 * - ID: MSL_CREATE_ID('G', 'L', 'E', 'S')
 * - Version: the GLSL version (e.g. 100, 300)
 * - GLSLES_VERSION is defined to the version number when preprocesssing.
 */
class MSL_COMPILE_EXPORT TargetGlsl : public Target
{
public:
	/**
	 * @brief Enum for the precisio
	 */
	enum class Precision
	{
		None,   ///< No precision set.
		Low,    ///< lowp
		Medium, ///< mediump
		High    ///< highp
	};

	/**
	 * @brief Constructs this with the version number.
	 * @param version The GLSL version number.
	 * @param isEs True if this is GLSL for OpenGL ES.
	 */
	TargetGlsl(std::uint32_t version, bool isEs);

	/**
	 * @brief Returns whether or not this is for OpenGL ES.
	 * @return True if for OpenGL ES.
	 */
	bool isEs() const;

	/**
	 * @brief Returns whether or not to remap the depth range from Vulkan to OpenGL. Defaults to
	 * true.
	 * @return True to remap depth range.
	 */
	bool getRemapDepthRange() const;

	/**
	 * @brief Sets whether or not to remap the depth range from Vulkan to OpenGL. Defaults to true.
	 * @param remap True to remap depth range.
	 */
	void setRemapDepthRange(bool remap);

	/**
	 * @brief Gets the default float precision. Defaults to Precision::Medum.
	 * @return The default float precision.
	 */
	Precision getDefaultFloatPrecision() const;

	/**
	 * @brief Sets the default float precision. Defaults to Precision::Medum.
	 * @param precision The default float precision.
	 */
	void setDefaultFloatPrecision(Precision precision);

	/**
	 * @brief Gets the default int precision. Defaults to Precision::High.
	 * @return The default int precision.
	 */
	Precision getDefaultIntPrecision() const;

	/**
	 * @brief Sets the default int precision. Defaults to Precision::High.
	 * @param precision The default int precision.
	 */
	void setDefaultIntPrecision(Precision precision);

	/**
	 * @brief Adds a header line to to be added to the final GLSL source for all pipeline stages.
	 * @param header The header line.
	 */
	void addHeaderLine(const std::string& header);

	/**
	 * @brief Adds a header line to to be added to the final GLSL source for a specific pipeline
	 * stage.
	 * @param stage The stage to add the header line for.
	 * @param header The header line.
	 */
	void addHeaderLine(Stage stage, std::string header);

	/**
	 * @brief Gets the header lines to be added to the final GLSL source.
	 * @param stage The stage to get the header lines for.
	 * @return The header lines.
	 */
	const std::vector<std::string>& getHeaderLines(Stage stage) const;

	/**
	 * @brief Clears the header lines.
	 */
	void clearHeaderLines();

	/**
	 * @brief Adds a required extension for all stages.
	 * @param extension The extension.
	 */
	void addRequiredExtension(const std::string& extension);

	/**
	 * @brief Adds a required extension for a specific stage stages.
	 * @param stage The stage to add the extension for.
	 * @param extension The extension.
	 */
	void addRequiredExtension(Stage stage, std::string extension);

	/**
	 * @brief Gets the require extensions.
	 * @param stage The stage to get the extensions for.
	 * @return The required extensions.
	 */
	const std::vector<std::string>& getRequiredExtensions(Stage stage) const;

	/**
	 * @brief Clears the required extensions.
	 */
	void clearRequiredExtensions();

	/**
	 * @brief Gets the tool command to run on the output GLSL.
	 * @param stage The stage the command is run on.
	 * @return The GLSL tool command.
	 */
	const std::string& getGlslToolCommand(Stage stage) const;

	/**
	 * @brief Sets the GLSL tool command to run on the output GLSL.
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
	 * @param stage The stage to run the command on.
	 * @param command The command to run.
	 */
	void setGlslToolCommand(Stage stage, std::string command);

	std::uint32_t getId() const override;
	std::uint32_t getVersion() const override;
	bool featureSupported(Feature feature) const override;
	std::vector<std::pair<std::string, std::string>> getExtraDefines() const override;

protected:
	bool crossCompile(std::vector<std::uint8_t>& data, Output& output, Stage stage,
		const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
		const std::string& fileName, std::size_t line, std::size_t column) override;

private:
	std::uint32_t m_version;
	bool m_es;

	bool m_remapDepthRange;
	Precision m_defaultFloatPrecision;
	Precision m_defaultIntPrecision;
	std::array<std::vector<std::string>, stageCount> m_headerLines;
	std::array<std::vector<std::string>, stageCount> m_requiredExtensions;
	std::array<std::string, stageCount> m_glslToolCommand;
};

} // namespace msl
