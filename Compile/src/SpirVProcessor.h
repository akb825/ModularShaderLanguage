/*
 * Copyright 2016-2019 Aaron Barany
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
#include <string>
#include <utility>
#include <vector>

namespace msl
{

using namespace compile;

class Output;

class MSL_COMPILE_EXPORT SpirVProcessor
{
public:
	enum class Strip
	{
		None,
		All,
		AllButReflection
	};

	struct InputOutput
	{
		std::string name;
		Type type;
		std::uint32_t structIndex;
		std::vector<std::uint32_t> arrayElements;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> memberLocations;
		bool patch;
		bool autoAssigned;
		std::uint32_t location;
		std::uint32_t component;
	};

	bool extract(Output& output, const std::string& fileName, std::size_t line, std::size_t column,
		const std::vector<std::uint32_t>& spirv, Stage stage);
	bool uniformsCompatible(Output& output, const SpirVProcessor& other) const;
	bool assignInputs(Output& output);
	bool assignOutputs(Output& output);
	bool linkInputs(Output& output, const SpirVProcessor& prevStage);
	std::vector<std::uint32_t> process(Strip strip, bool dummyBindings) const;

	Stage stage;
	std::string fileName;
	std::size_t line;
	std::size_t column;
	const std::vector<std::uint32_t>* spirv;

	std::vector<Struct> structs;
	std::vector<std::uint32_t> structIds;
	std::vector<Uniform> uniforms;
	std::vector<std::uint32_t> uniformIds;
	std::vector<InputOutput> inputs;
	std::vector<std::uint32_t> inputIds;
	std::vector<InputOutput> outputs;
	std::vector<std::uint32_t> outputIds;
	std::uint32_t pushConstantStruct = unknown;
	std::array<std::uint32_t, 3> computeLocalSize = {1, 1, 1};
};

} // namespace msl
