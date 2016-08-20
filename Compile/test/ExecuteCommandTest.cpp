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
#include <gtest/gtest.h>

namespace msl
{

TEST(ExecuteCommandTest, CommandNotFound)
{
	Output output;
	ExecuteCommand command;
	EXPECT_FALSE(command.execute(output, "asdf"));
	ASSERT_EQ(2U, output.getMessages().size());
#if MSL_WINDOWS
	EXPECT_EQ("output from running command: asdf\n'asdf' is not recognized as an internal or "
		"external command,\noperable program or batch file.",
		output.getMessages()[0].message);
#else
	EXPECT_EQ("output from running command: asdf\nsh: asdf: command not found",
		output.getMessages()[0].message);
#endif
}

#if !MSL_WINDOWS

TEST(ExecuteCommandTest, InputToOutput)
{
	Output output;
	ExecuteCommand command;
	command.getInput() << "testing 123";
	EXPECT_TRUE(command.execute(output, "cat $input | tee $output"));
	std::string outputStr(std::istreambuf_iterator<char>(command.getOutput().rdbuf()),
		std::istreambuf_iterator<char>());
	EXPECT_EQ("testing 123", outputStr);
	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ("output from running command: cat $input | tee $output\ntesting 123",
		output.getMessages()[0].message);
}

#endif

} // namespace msl
