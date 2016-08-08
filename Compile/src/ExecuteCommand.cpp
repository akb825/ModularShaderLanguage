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

#include "ExecuteCommand.h"
#include <MSL/Compile/Output.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <cstdio>
#include <sstream>

#if MSL_WINDOWS
#define popen _popen
#define pclose _pclose
#endif

namespace msl
{

ExecuteCommand::ExecuteCommand()
{
	m_inputFileName =
		(boost::filesystem::temp_directory_path()/boost::filesystem::unique_path()).string();
	m_outputFileName =
		(boost::filesystem::temp_directory_path()/boost::filesystem::unique_path()).string();

	m_input.open(m_inputFileName, std::ofstream::binary);
}

ExecuteCommand::~ExecuteCommand()
{
	m_input.close();
	boost::filesystem::remove(m_inputFileName);
	m_output.close();
	boost::filesystem::remove(m_outputFileName);
}

bool ExecuteCommand::execute(Output& output, const std::string& command)
{
	std::string finalCommand = command;
	boost::algorithm::replace_all(finalCommand, "$input", m_inputFileName);
	boost::algorithm::replace_all(finalCommand, "$output", m_outputFileName);
	finalCommand += " 2>&1";

	m_input.close();

	FILE* pipe = popen(finalCommand.c_str(), "r");
	if (!pipe)
	{
		output.addMessage(Output::Level::Error, "", 0, 0, false,
			"could not execute command: " + command);
		return false;
	}

	const unsigned int bufferSize = 1024;
	char buffer[bufferSize];
	std::vector<char> executeOutput;
	while (!feof(pipe))
	{
		std::size_t readSize = fread(buffer, sizeof(char), bufferSize, pipe);
		executeOutput.insert(executeOutput.end(), buffer, buffer + readSize);
	}
	int exitCode = pclose(pipe);

	std::string outputStr(executeOutput.begin(), executeOutput.end());
	boost::algorithm::trim(outputStr);

	if (!outputStr.empty())
		output.addMessage(Output::Level::Info, "", 0, 0, false, outputStr);

	if (exitCode != 0)
	{
		std::stringstream stream;
		stream << "command failed with exit code " << exitCode << ": " << command;
		output.addMessage(Output::Level::Error, "", 0, 0, false, stream.str());
	}

	m_output.open(m_outputFileName, std::ifstream::binary);
	return exitCode == 0;
}

} // namespace
