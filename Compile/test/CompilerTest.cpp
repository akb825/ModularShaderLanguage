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
#include "Compiler.h"
#include "Parser.h"
#include "Preprocessor.h"
#include "StandAlone/ResourceLimits.h"
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

TEST_F(CompilerTest, CompleteShader)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompleteShader.msl").string();

	Parser parser;
	Preprocessor preprocessor;
	Compiler compiler;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output, shaderName));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;
	bool compiledStage = false;
	for (unsigned int i = 0; i < Parser::stageCount; ++i)
	{
		if (pipeline.entryPoints[i].empty())
			continue;

		auto stage = static_cast<Parser::Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl = parser.createShaderString(lineMappings, pipeline, stage);
		EXPECT_TRUE(compiler.compile(stages, output, shaderName, glsl, lineMappings, stage,
			pipeline.entryPoints[i], glslang::DefaultTBuiltInResource));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	glslang::TProgram program;
	EXPECT_TRUE(compiler.link(program, output, pipeline, stages));

	bool assembledStage = false;
	for (unsigned int i = 0; i < Parser::stageCount; ++i)
	{
		if (!stages.shaders[i])
			continue;

		EXPECT_FALSE(compiler.assemble(output, program, static_cast<Parser::Stage>(i),
			pipeline).empty());
		assembledStage = true;
	}
	EXPECT_TRUE(assembledStage);
	EXPECT_TRUE(output.getMessages().empty());
}

TEST_F(CompilerTest, CompileError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompileError.msl").string();

	Parser parser;
	Preprocessor preprocessor;
	Compiler compiler;
	Output output;
	preprocessor.addIncludePath(inputDir.string());
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output, shaderName));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;

	auto stage = Parser::Stage::Fragment;
	auto stageIndex = static_cast<unsigned int>(stage);
	std::vector<Parser::LineMapping> lineMappings;
	std::string glsl = parser.createShaderString(lineMappings, pipeline, stage);
	EXPECT_FALSE(compiler.compile(stages, output, shaderName, glsl, lineMappings, stage,
		pipeline.entryPoints[stageIndex], glslang::DefaultTBuiltInResource));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(inputDir/"CompileError.mslh", messages[0].file);
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'inputss' : undeclared identifier", messages[0].message);
}

TEST_F(CompilerTest, CompileWarning)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompileWarning.msl").string();

	Parser parser;
	Preprocessor preprocessor;
	Compiler compiler;
	Output output;
	preprocessor.addIncludePath(inputDir.string());
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output, shaderName));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;

	auto stage = Parser::Stage::Fragment;
	auto stageIndex = static_cast<unsigned int>(stage);
	std::vector<Parser::LineMapping> lineMappings;
	std::string glsl = parser.createShaderString(lineMappings, pipeline, stage);
	EXPECT_TRUE(compiler.compile(stages, output, shaderName, glsl, lineMappings, stage,
		pipeline.entryPoints[stageIndex], glslang::DefaultTBuiltInResource));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Warning, messages[0].level);
	EXPECT_EQ(inputDir/"CompileWarning.mslh", messages[0].file);
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'switch' : last case/default label not followed by statements", messages[0].message);
}

TEST_F(CompilerTest, LinkerError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"LinkError.msl").string();

	Parser parser;
	Preprocessor preprocessor;
	Compiler compiler;
	Output output;
	preprocessor.addIncludePath(inputDir.string());
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output, shaderName));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;
	bool compiledStage = false;
	for (unsigned int i = 0; i < Parser::stageCount; ++i)
	{
		if (pipeline.entryPoints[i].empty())
			continue;

		auto stage = static_cast<Parser::Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl = parser.createShaderString(lineMappings, pipeline, stage);
		EXPECT_TRUE(compiler.compile(stages, output, shaderName, glsl, lineMappings, stage,
			pipeline.entryPoints[i], glslang::DefaultTBuiltInResource));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	glslang::TProgram program;
	EXPECT_FALSE(compiler.link(program, output, pipeline, stages));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(inputDir/"LinkError.mslh", messages[0].file);
	EXPECT_EQ(5U, messages[0].line);
	EXPECT_EQ("Linking fragment stage: Missing entry point: "
		"Each stage requires one \"void main()\" entry point", messages[0].message);
}

} // namespace msl
