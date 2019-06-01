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

/**
 * @file
 * @brief Target implementation for compiling to SPIR-V.
 */

namespace msl
{

/**
 * @brief Target implementation for compiling to SPIR-V.
 *
 * The following are set for this target:
 * - ID: MSL_CREATE_ID('S', 'P', 'R', 'V')
 * - Version: 1
 * - SPIRV_VERSION is defined to the version number when preprocessing.
 */
class MSL_COMPILE_EXPORT TargetSpirV : public Target
{
public:
	std::uint32_t getId() const override;
	std::uint32_t getVersion() const override;
	bool featureSupported(Feature feature) const override;
	std::vector<std::pair<std::string, std::string>> getExtraDefines() const override;

protected:
	bool needsReflectionNames() const override;
	bool crossCompile(std::vector<std::uint8_t>& data, Output& output,
		const std::string& fileName, std::size_t line, std::size_t column,
		const std::array<bool, compile::stageCount>& pipelineStages, compile::Stage stage,
		const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
		const std::vector<compile::Uniform>& uniforms,
		std::vector<std::uint32_t>& uniformIds) override;
};

} // namespace msl
