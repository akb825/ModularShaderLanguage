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

#include <MSL/Compile/CompiledResult.h>

namespace msl
{

CompiledResult::CompiledResult()
	: m_target(nullptr)
	, m_targetId(0)
	, m_targetVersion(0)
{
}

std::size_t CompiledResult::addShader(std::vector<uint8_t> shader)
{
	for (std::size_t i = 0; i < m_shaders.size(); ++i)
	{
		if (m_shaders[i] == shader)
			return i;
	}

	m_shaders.push_back(std::move(shader));
	return m_shaders.size() - 1;
}

} // namespace msl
