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
#include <map>

/**
 * @file
 * @brief Target implementation for compiling to Metal.
 */

namespace msl
{

/**
 * @brief Target implementation for compiling to Metal, both for Mac OS X and iOS.
 *
 * When compiling for Mac OS X, the following will be set:
 * - ID: MSL_CREATE_ID('M', 'T', 'L', 'X')
 * - Version: the metal version times 10. For example, 10 is version 1.0, 11 is version 1.1.
 * - METAL_OSX_VERSION is defined to the version number when preprocesssing.
 *
 * When compiling for iOS, the following will be set:
 * - ID: MSL_CREATE_ID('M', 'T', 'L', 'I')
 * - Version: the metal version times 10. For example, 10 is version 1.0, 11 is version 1.1.
 * - METAL_IOS_VERSION is defined to the version number when preprocesssing.
 *
 * The shared data contains the MTLLibrary data. The per-shader data is the name of the function
 * within the library.
 */
class MSL_COMPILE_EXPORT TargetMetal : public Target
{
public:
	/**
	 * @brief Enum for the platform to target.
	 */
	enum class Platform
	{
		MacOS,       ///< Target macOS.
		iOS,         ///< Target iOS.
		iOSSimulator ///< Target iOS simulator.
	};

	/**
	 * @brief Constructs this with the version number.
	 * @param version The Metal version number.
	 * @param platform The platform to target.
	 */
	TargetMetal(std::uint32_t version, Platform platform);

	~TargetMetal();
	TargetMetal(const TargetMetal&) = delete;
	TargetMetal& operator=(const TargetMetal&) = delete;

	/**
	 * @brief Gets the platform that's targetd.
	 * @return The targeted platform.
	 */
	Platform getPlatform() const;

	std::uint32_t getId() const override;
	std::uint32_t getVersion() const override;
	bool featureSupported(Feature feature) const override;
	std::vector<std::pair<std::string, std::string>> getExtraDefines() const override;

protected:
	void willCompile() override;
	bool crossCompile(std::vector<std::uint8_t>& data, Output& output, const std::string& fileName,
		std::size_t line, std::size_t column,
		const std::array<bool, compile::stageCount>& pipelineStages, compile::Stage stage,
		const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
		const std::vector<compile::Uniform>& uniforms,
		std::vector<std::uint32_t>& uniformIds) override;

private:
	std::string getSDK() const;

	std::uint32_t m_version;
	Platform m_platform;
};

} // namespace msl
