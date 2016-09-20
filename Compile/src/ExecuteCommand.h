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
#include <fstream>
#include <string>

namespace msl
{

class Output;

// Export for tests.
class MSL_COMPILE_EXPORT ExecuteCommand
{
public:
	ExecuteCommand();
	~ExecuteCommand();

	ExecuteCommand(const ExecuteCommand&) = delete;
	ExecuteCommand& operator=(const ExecuteCommand&) = delete;

	std::ofstream& getInput()
	{
		return m_input;
	}

	std::ifstream& getOutput()
	{
		return m_output;
	}

	const std::string& getOutputFileName() const
	{
		return m_inputFileName;
	}

	bool execute(Output& output, const std::string& command);

private:
	std::string m_inputFileName;
	std::ofstream m_input;

	std::string m_outputFileName;
	std::ifstream m_output;
};

} // namespace msl
