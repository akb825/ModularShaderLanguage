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

#include <MSL/Compile/TargetSpirV.h>
#include <cstring>
#include <sstream>

namespace msl
{

std::uint32_t TargetSpirV::getId() const
{
	return MSL_CREATE_ID('S', 'P', 'R', 'V');
}

std::uint32_t TargetSpirV::getVersion() const
{
	return 1;
}

bool TargetSpirV::featureSupported(Feature) const
{
	return true;
}

std::vector<std::pair<std::string, std::string>> TargetSpirV::getExtraDefines() const
{
	std::stringstream stream;
	stream << getVersion();
	return {{"SPIRV_VERSION", stream.str()}};
}

bool TargetSpirV::needsReflectionNames() const
{
	return false;
}

bool TargetSpirV::crossCompile(std::vector<std::uint8_t>& data, Output&, const std::string&,
	std::size_t, std::size_t, compile::Stage, const std::vector<std::uint32_t>& spirv,
	const std::string&, const std::vector<compile::Uniform>& uniforms,
	std::vector<std::uint32_t>& uniformIds)
{
	data.resize(spirv.size()*sizeof(std::uint32_t));
	std::memcpy(data.data(), spirv.data(), data.size());
	return true;
}

} // namespace msl
