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
#include "Parser.h"
#include "Preprocessor.h"
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>

namespace msl
{

TEST(ParserMultisampleStateTest, SampledShadingEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {sample_shading_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.multisampleState.sampleShadingEnable);
	}

	{
		std::stringstream stream("pipeline Test {sample_shading_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.multisampleState.sampleShadingEnable);
	}

	{
		std::stringstream stream("pipeline Test {sample_shading_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(40U, messages[0].column);
		EXPECT_EQ("invalid boolean value: asdf", messages[0].message);
	}
}

TEST(ParserMultisampleStateTest, MinSampleShading)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {min_sample_shading = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(-3.0f,
			pipelines[0].renderState.multisampleState.minSampleShading);
	}

	{
		std::stringstream stream("pipeline Test {min_sample_shading = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.3e4f,
			pipelines[0].renderState.multisampleState.minSampleShading);
	}

	{
		std::stringstream stream("pipeline Test {min_sample_shading = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(37U, messages[0].column);
		EXPECT_EQ("invalid float value: asdf", messages[0].message);
	}
}

TEST(ParserMultisampleStateTest, SampleMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {sample_mask = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U,
			pipelines[0].renderState.multisampleState.sampleMask);
	}

	{
		std::stringstream stream("pipeline Test {sample_mask = 0123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0123U,
			pipelines[0].renderState.multisampleState.sampleMask);
	}

	{
		std::stringstream stream("pipeline Test {sample_mask = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.multisampleState.sampleMask);
	}

	{
		std::stringstream stream("pipeline Test {sample_mask = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(30U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserMultisampleStateTest, AlphaToCoverageEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {alpha_to_coverage_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.multisampleState.alphaToCoverageEnable);
	}

	{
		std::stringstream stream("pipeline Test {alpha_to_coverage_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.multisampleState.alphaToCoverageEnable);
	}

	{
		std::stringstream stream("pipeline Test {alpha_to_coverage_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(43U, messages[0].column);
		EXPECT_EQ("invalid boolean value: asdf", messages[0].message);
	}
}

TEST(ParserMultisampleStateTest, AlphaToOneEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {alpha_to_one_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.multisampleState.alphaToOneEnable);
	}

	{
		std::stringstream stream("pipeline Test {alpha_to_one_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.multisampleState.alphaToOneEnable);
	}

	{
		std::stringstream stream("pipeline Test {alpha_to_one_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(38U, messages[0].column);
		EXPECT_EQ("invalid boolean value: asdf", messages[0].message);
	}
}

} // namespace msl
