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
#include <MSL/Compile/Types.h>
#include <cstdint>
#include <string>
#include <vector>

namespace msl
{

class SpirVProcessor
{
public:

	explicit SpirVProcessor(const std::vector<std::uint32_t>& spirv);

	std::vector<Struct> structs;
	std::vector<std::uint32_t> structIds;
	std::vector<Uniform> uniforms;
	std::vector<std::uint32_t> uniformIds;
	std::vector<Attribute> inputs;
	std::vector<std::uint32_t> inputIds;
	std::vector<Attribute> outputs;
	std::vector<std::uint32_t> outputIds;
	std::uint32_t pushConstantStruct;
};

} // namespace msl
