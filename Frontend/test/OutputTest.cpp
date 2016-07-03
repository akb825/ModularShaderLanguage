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

#include <MSL/Frontend/Output.h>
#include <gtest/gtest.h>

namespace msl
{

TEST(OutputTest, AddMessage)
{
	Output output;
	output.addMessage(Output::Message(Output::Level::Warning, "test1", 1, 2, false, "message 1"));
	output.addMessage(Output::Level::Error, "test2", 3, 4, true, "message 2");

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(2, messages.size());
	EXPECT_EQ(Output::Level::Warning, messages[0].level);
	EXPECT_EQ("test1", messages[0].file);
	EXPECT_EQ(1, messages[0].line);
	EXPECT_EQ(2, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("message 1", messages[0].message);

	EXPECT_EQ(Output::Level::Error, messages[1].level);
	EXPECT_EQ("test2", messages[1].file);
	EXPECT_EQ(3, messages[1].line);
	EXPECT_EQ(4, messages[1].column);
	EXPECT_TRUE(messages[1].continued);
	EXPECT_EQ("message 2", messages[1].message);
}

TEST(OutputTest, Clear)
{
	Output output;
	output.addMessage(Output::Message(Output::Level::Warning, "test1", 1, 2, false, "message 1"));
	output.addMessage(Output::Level::Error, "test2", 3, 4, true, "message 2");

	EXPECT_FALSE(output.empty());
	output.clear();
	EXPECT_TRUE(output.empty());
}

TEST(OutputTest, CountWarningsErrors)
{
	Output output;
	output.addMessage(Output::Level::Error, "test1", 1, 2, false, "error 1");
	output.addMessage(Output::Level::Warning, "test1", 1, 2, false, "warning 1");
	output.addMessage(Output::Level::Error, "test1", 1, 2, false, "error 2");
	output.addMessage(Output::Level::Error, "test1", 2, 0, true, "error 2");
	output.addMessage(Output::Level::Warning, "test2", 4, 0, false, "warning 2");
	output.addMessage(Output::Level::Warning, "test2", 5, 0, false, "warning 3");
	output.addMessage(Output::Level::Warning, "test2", 6, 0, true, "warning 3");
	output.addMessage(Output::Level::Warning, "test2", 7, 0, true, "warning 3");
	output.addMessage(Output::Level::Warning, "test2", 8, 0, false, "warning 4");

	EXPECT_EQ(4U, output.getWarningCount());
	EXPECT_EQ(2U, output.getErrorCount());
}

} // namespace msl
