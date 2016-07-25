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

#include "Helpers.h"
#include <MSL/Compile/Output.h>
#include "Preprocessor.h"
#include <gtest/gtest.h>

namespace msl
{

TEST(PreprocessorTest, NotFound)
{
	Preprocessor preprocessor;
	TokenList tokens;
	Output output;
	EXPECT_FALSE(preprocessor.preprocess(tokens, output, "NotFound"));
	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ("could not find file: NotFound", output.getMessages()[0].message);
}

TEST(PreprocessorTest, SimpleFile)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Preprocessor preprocessor;
	preprocessor.addIncludePath(inputDir.string());
	preprocessor.addDefine("TEST", "1");

	TokenList tokens;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(tokens, output, (inputDir/"Simple.msl").string()));
	EXPECT_EQ(readFile(outputDir/"Simple.msl"), tokensToString(tokens));
}

TEST(PreprocessorTest, PreprocError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Preprocessor preprocessor;
	preprocessor.addIncludePath(inputDir.string());

	TokenList tokens;
	Output output;
	std::string fileName = (inputDir/"PreprocError.msl").string();
	EXPECT_FALSE(preprocessor.preprocess(tokens, output, fileName));
	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(Output::Level::Error, output.getMessages()[0].level);
	EXPECT_EQ(fileName, output.getMessages()[0].file);
	EXPECT_EQ(2U, output.getMessages()[0].line);
	EXPECT_EQ(1U, output.getMessages()[0].column);
	EXPECT_EQ("illegal macro redefinition: a", output.getMessages()[0].message);
}

TEST(PreprocessorTest, IncludeError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Preprocessor preprocessor;
	preprocessor.addIncludePath(inputDir.string());

	TokenList tokens;
	Output output;
	std::string fileName = (inputDir/"IncludeError.msl").string();
	EXPECT_FALSE(preprocessor.preprocess(tokens, output, fileName));
	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(Output::Level::Error, output.getMessages()[0].level);
	EXPECT_EQ(fileName, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(1U, output.getMessages()[0].column);
	EXPECT_EQ("could not find include file: asdf.mslh", output.getMessages()[0].message);
}

} // namespace msl
