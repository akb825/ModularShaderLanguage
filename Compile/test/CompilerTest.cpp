/*
 * Copyright 2016-2025 Aaron Barany
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
#include "Compiler.h"
#include "Parser.h"
#include "Preprocessor.h"
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>

namespace msl
{

class CompilerTest : public testing::Test
{
public:
	void SetUp() override
	{
		Compiler::initialize();
	}

	void TearDown() override
	{
		Compiler::shutdown();
	}
};

static constexpr std::uint32_t spirvVersion = 0x10000;

TEST_F(CompilerTest, CompleteShader)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;
	bool compiledStage = false;
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));

	bool assembledStage = false;
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (!stages.shaders[i])
			continue;

		EXPECT_FALSE(Compiler::assemble(output, program, static_cast<Stage>(i), pipeline).empty());
		assembledStage = true;
	}
	EXPECT_TRUE(assembledStage);
	EXPECT_TRUE(output.getMessages().empty());
}

TEST_F(CompilerTest, CompileError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompileError.msl");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(pathStr(inputDir));
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;

	auto stage = Stage::Fragment;
	std::vector<Parser::LineMapping> lineMappings;
	std::string glsl =
		parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
	EXPECT_FALSE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
		Compiler::getDefaultResources(), spirvVersion));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"CompileError.mslh")));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'inputss' : undeclared identifier", messages[0].message);
}

TEST_F(CompilerTest, CompileErrorWithEarlyFragmentTests)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompileError.msl");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(pathStr(inputDir));
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;

	auto stage = Stage::Fragment;
	std::vector<Parser::LineMapping> lineMappings;
	std::string glsl =
		parser.createShaderString(lineMappings, output, pipeline, stage, false, true);
	EXPECT_FALSE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
		Compiler::getDefaultResources(), spirvVersion));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"CompileError.mslh")));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'inputss' : undeclared identifier", messages[0].message);
}

TEST_F(CompilerTest, CompileWarning)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompileWarning.msl");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(pathStr(inputDir));
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;

	auto stage = Stage::Fragment;
	std::vector<Parser::LineMapping> lineMappings;
	std::string glsl =
		parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
	EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
		Compiler::getDefaultResources(), spirvVersion));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Warning, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"CompileWarning.mslh")));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'switch' : last case/default label not followed by statements", messages[0].message);
}

TEST_F(CompilerTest, MissingEntryPoint)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"MissingEntryPoint.msl");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(pathStr(inputDir));
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;
	bool compiledStage = false;
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		if (stage == Stage::Fragment)
			EXPECT_TRUE(glsl.empty());
		else
			EXPECT_FALSE(glsl.empty());
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"MissingEntryPoint.mslh")));
	EXPECT_EQ(8U, messages[0].line);
	EXPECT_EQ("entry point 'fragShader' not found", messages[0].message);
}

TEST_F(CompilerTest, DuplicateEntryPoint)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"DuplicateEntryPoint.msl");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(pathStr(inputDir));
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;
	bool compiledStage = false;
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		if (stage == Stage::Fragment)
			EXPECT_TRUE(glsl.empty());
		else
			EXPECT_FALSE(glsl.empty());
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"DuplicateEntryPoint.mslh")));
	EXPECT_EQ(8U, messages[0].line);
	EXPECT_EQ("entry point 'fragShader' found multiple times", messages[0].message);
}

} // namespace msl
