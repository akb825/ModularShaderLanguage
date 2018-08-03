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

static unsigned int unsetInt = unknown;

TEST(ParserDepthStencilStateTest, SampledShadingEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_test_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True,
			pipelines[0].renderState.depthStencilState.depthTestEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_test_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False,
			pipelines[0].renderState.depthStencilState.depthTestEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_test_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(36U, messages[0].column);
		EXPECT_EQ("invalid boolean value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, DepthWriteEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_write_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True,
			pipelines[0].renderState.depthStencilState.depthWriteEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_write_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False,
			pipelines[0].renderState.depthStencilState.depthWriteEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_write_enable = asdf;}");
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
		EXPECT_EQ("invalid boolean value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, DepthCompareOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_compare_op = never ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::Never,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = less;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::Less,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = equal;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::Equal,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = less_or_equal;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::LessOrEqual,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = greater;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::Greater,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = not_equal;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::NotEqual,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = greater_or_equal;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::GreaterOrEqual,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = always;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CompareOp::Always,
			pipelines[0].renderState.depthStencilState.depthCompareOp);
	}

	{
		std::stringstream stream("pipeline Test {depth_compare_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(35U, messages[0].column);
		EXPECT_EQ("invalid compare op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, DepthBoundsTestEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_bounds_test_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True,
			pipelines[0].renderState.depthStencilState.depthBoundsTestEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_bounds_test_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False,
			pipelines[0].renderState.depthStencilState.depthBoundsTestEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_bounds_test_enable = asdf;}");
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

TEST(ParserDepthStencilStateTest, StencilTestEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {stencil_test_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True,
			pipelines[0].renderState.depthStencilState.stencilTestEnable);
	}

	{
		std::stringstream stream("pipeline Test {stencil_test_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False,
			pipelines[0].renderState.depthStencilState.stencilTestEnable);
	}

	{
		std::stringstream stream("pipeline Test {stencil_test_enable = asdf;}");
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

TEST(ParserDepthStencilStateTest, StencilFailOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {stencil_fail_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = zero;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Zero,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::Zero,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = replace;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Replace,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::Replace,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = decrement_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::DecrementAndClamp,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::DecrementAndClamp,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = invert;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Invert,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::Invert,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = increment_and_wrap;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::IncrementAndWrap,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::IncrementAndWrap,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = decrement_and_wrap;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::DecrementAndWrap,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::DecrementAndWrap,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_fail_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(34U, messages[0].column);
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, StencilPassOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {stencil_pass_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.frontStencil.passOp);
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.backStencil.passOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_pass_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.frontStencil.passOp);
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.backStencil.passOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_pass_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(34U, messages[0].column);
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, StencilDepthPassOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {stencil_depth_fail_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.frontStencil.depthFailOp);
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.backStencil.depthFailOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_depth_fail_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.frontStencil.depthFailOp);
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.backStencil.depthFailOp);
	}

	{
		std::stringstream stream("pipeline Test {stencil_depth_fail_op = asdf;}");
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
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, StencilCompareMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {stencil_compare_mask = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.frontStencil.compareMask);
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.backStencil.compareMask);
	}

	{
		std::stringstream stream("pipeline Test {stencil_compare_mask = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.frontStencil.compareMask);
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.backStencil.compareMask);
	}

	{
		std::stringstream stream("pipeline Test {stencil_compare_mask = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(39U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, StencilWriteMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {stencil_write_mask = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.frontStencil.writeMask);
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.backStencil.writeMask);
	}

	{
		std::stringstream stream("pipeline Test {stencil_write_mask = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.frontStencil.writeMask);
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.backStencil.writeMask);
	}

	{
		std::stringstream stream("pipeline Test {stencil_write_mask = asdf;}");
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
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, StencilReference)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {stencil_reference = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.frontStencil.reference);
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.backStencil.reference);
	}

	{
		std::stringstream stream("pipeline Test {stencil_reference = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.frontStencil.reference);
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.backStencil.reference);
	}

	{
		std::stringstream stream("pipeline Test {stencil_reference = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(36U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, FrontStencilFailOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {front_stencil_fail_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_fail_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_fail_op = asdf;}");
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
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, FrontStencilPassOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {front_stencil_pass_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.frontStencil.passOp);
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.backStencil.passOp);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_pass_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.frontStencil.passOp);
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.backStencil.passOp);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_pass_op = asdf;}");
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
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, FrontStencilDepthPassOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {front_stencil_depth_fail_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.frontStencil.depthFailOp);
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.backStencil.depthFailOp);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_depth_fail_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.frontStencil.depthFailOp);
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.backStencil.depthFailOp);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_depth_fail_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(46U, messages[0].column);
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, FrontStencilCompareMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {front_stencil_compare_mask = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.frontStencil.compareMask);
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.backStencil.compareMask);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_compare_mask = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.frontStencil.compareMask);
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.backStencil.compareMask);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_compare_mask = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(45U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, FrontStencilWriteMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {front_stencil_write_mask = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.frontStencil.writeMask);
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.backStencil.writeMask);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_write_mask = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.frontStencil.writeMask);
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.backStencil.writeMask);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_write_mask = asdf;}");
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
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, FrontStencilReference)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {front_stencil_reference = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.frontStencil.reference);
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.backStencil.reference);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_reference = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.frontStencil.reference);
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.backStencil.reference);
	}

	{
		std::stringstream stream("pipeline Test {front_stencil_reference = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(42U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, BackStencilFailOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {back_stencil_fail_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_fail_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.frontStencil.failOp);
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.backStencil.failOp);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_fail_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(39U, messages[0].column);
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, BackStencilPassOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {back_stencil_pass_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.frontStencil.passOp);
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.backStencil.passOp);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_pass_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.frontStencil.passOp);
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.backStencil.passOp);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_pass_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(39U, messages[0].column);
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, BackStencilDepthPassOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {back_stencil_depth_fail_op = keep ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.frontStencil.depthFailOp);
		EXPECT_EQ(StencilOp::Keep,
			pipelines[0].renderState.depthStencilState.backStencil.depthFailOp);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_depth_fail_op = increment_and_clamp;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(StencilOp::Unset,
			pipelines[0].renderState.depthStencilState.frontStencil.depthFailOp);
		EXPECT_EQ(StencilOp::IncrementAndClamp,
			pipelines[0].renderState.depthStencilState.backStencil.depthFailOp);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_depth_fail_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(45U, messages[0].column);
		EXPECT_EQ("invalid stencil op value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, BackStencilCompareMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {back_stencil_compare_mask = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.frontStencil.compareMask);
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.backStencil.compareMask);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_compare_mask = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.frontStencil.compareMask);
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.backStencil.compareMask);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_compare_mask = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(44U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, BackStencilWriteMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {back_stencil_write_mask = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.frontStencil.writeMask);
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.backStencil.writeMask);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_write_mask = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.frontStencil.writeMask);
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.backStencil.writeMask);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_write_mask = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(42U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, BackStencilReference)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {back_stencil_reference = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.frontStencil.reference);
		EXPECT_EQ(123U,
			pipelines[0].renderState.depthStencilState.backStencil.reference);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_reference = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(unsetInt,
			pipelines[0].renderState.depthStencilState.frontStencil.reference);
		EXPECT_EQ(0xABCD,
			pipelines[0].renderState.depthStencilState.backStencil.reference);
	}

	{
		std::stringstream stream("pipeline Test {back_stencil_reference = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(41U, messages[0].column);
		EXPECT_EQ("invalid int value: asdf", messages[0].message);
	}
}

TEST(ParserDepthStencilStateTest, MinDepthBounds)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {min_depth_bounds = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(-3.0f,
			pipelines[0].renderState.depthStencilState.minDepthBounds);
	}

	{
		std::stringstream stream("pipeline Test {min_depth_bounds = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.3e4f,
			pipelines[0].renderState.depthStencilState.minDepthBounds);
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

TEST(ParserDepthStencilStateTest, MaxDepthBounds)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {max_depth_bounds = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(-3.0f,
			pipelines[0].renderState.depthStencilState.maxDepthBounds);
	}

	{
		std::stringstream stream("pipeline Test {max_depth_bounds = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.3e4f,
			pipelines[0].renderState.depthStencilState.maxDepthBounds);
	}

	{
		std::stringstream stream("pipeline Test {max_depth_bounds = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(35U, messages[0].column);
		EXPECT_EQ("invalid float value: asdf", messages[0].message);
	}
}

} // namespace msl
