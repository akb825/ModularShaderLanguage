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
#include <MSL/Compile/Types.h>
#include <cstdint>
#include <map>
#include <ostream>
#include <vector>

/**
 * @file
 * @brief Class that stores the compiled result for a shader.
 */

namespace msl
{

class Target;

/**
 * @brief Class that stores the compiled result for a shader.
 *
 * This contains the compiled result and reflection information for the uniforms and buffers
 * for each pipeline.
 */
class MSL_COMPILE_EXPORT CompiledResult
{
public:
	/**
	 * @brief Constant for the file version.
	 */
	static const std::uint32_t version = 0;

	CompiledResult();

	/**
	 * @brief Gets the target ID.
	 * @return The target ID.
	 */
	std::uint32_t getTargetId() const;

	/**
	 * @brief Gets the target version.
	 * @return The target version.
	 */
	std::uint32_t getTargetVersion() const;

	/**
	 * @brief Gets the pipelines.
	 *
	 * The first element is the pipeline name, the second element contians the pipeline itself.
	 *
	 * @return The pipelines.
	 */
	inline const std::map<std::string, compile::Pipeline>& getPipelines() const;

	/**
	 * @brief Gets the shaders.
	 *
	 * The pipeline stage indices can be used to index into this vector.
	 *
	 * @return The shaders.
	 */
	inline const std::vector<std::vector<uint8_t>>& getShaders() const;

	/**
	 * @brief Gets the shared data for all the shaders.
	 * @return The shared data.
	 */
	inline const std::vector<std::uint8_t>& getSharedData() const;

	/**
	 * @brief Saves the compiled shader to a stream.
	 * @param stream The stream to save to.
	 * @return False if nothing was compiled.
	 */
	bool save(std::ostream& stream) const;

	/**
	 * @brief Saves the compiled shader to a file.
	 * @param fileName The name of the file to save to.
	 * @return False if the file couldn't be opened.
	 */
	bool save(const std::string& fileName) const;

private:
	friend class Target;

	std::size_t addShader(std::vector<std::uint8_t> shader, bool dontRemoveDuplicates);

	const Target* m_target;

	// Use a map to ensure consistent ordering.
	std::map<std::string, compile::Pipeline> m_pipelines;
	std::vector<std::vector<std::uint8_t>> m_shaders;
	std::vector<std::uint8_t> m_sharedData;
};

inline const std::map<std::string, compile::Pipeline>& CompiledResult::getPipelines() const
{
	return m_pipelines;
}

inline const std::vector<std::vector<uint8_t>>& CompiledResult::getShaders() const
{
	return m_shaders;
}

inline const std::vector<std::uint8_t>& CompiledResult::getSharedData() const
{
	return m_sharedData;
}

} // namespace msl
