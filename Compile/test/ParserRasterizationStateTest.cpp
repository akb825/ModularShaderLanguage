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

#include "Helpers.h"
#include <MSL/Compile/Output.h>
#include "Parser.h"
#include "Preprocessor.h"
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>

namespace msl
{

TEST(ParserRasterizationStateTest, DepthClampEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_clamp_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.rasterizationState.depthClampEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_clamp_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.rasterizationState.depthClampEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_clamp_enable = 1;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.rasterizationState.depthClampEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_clamp_enable = 0;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.rasterizationState.depthClampEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_clamp_enable = asdf;}");
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
		EXPECT_EQ("invalid boolean value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, RasterizerDiscardEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {rasterizer_discard_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.rasterizationState.rasterizerDiscardEnable);
	}

	{
		std::stringstream stream("pipeline Test {rasterizer_discard_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.rasterizationState.rasterizerDiscardEnable);
	}

	{
		std::stringstream stream("pipeline Test {rasterizer_discard_enable = asdf;}");
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
		EXPECT_EQ("invalid boolean value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, PolygonMode)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {polygon_mode = fill ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(PolygonMode::Fill, pipelines[0].renderState.rasterizationState.polygonMode);
	}

	{
		std::stringstream stream("pipeline Test {polygon_mode = line;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(PolygonMode::Line, pipelines[0].renderState.rasterizationState.polygonMode);
	}

	{
		std::stringstream stream("pipeline Test {polygon_mode = point;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(PolygonMode::Point, pipelines[0].renderState.rasterizationState.polygonMode);
	}

	{
		std::stringstream stream("pipeline Test {polygon_mode = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(31U, messages[0].column);
		EXPECT_EQ("invalid polygon mode value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, CullMode)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {cull_mode = none ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CullMode::None, pipelines[0].renderState.rasterizationState.cullMode);
	}

	{
		std::stringstream stream("pipeline Test {cull_mode = front;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CullMode::Front, pipelines[0].renderState.rasterizationState.cullMode);
	}

	{
		std::stringstream stream("pipeline Test {cull_mode = back;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CullMode::Back, pipelines[0].renderState.rasterizationState.cullMode);
	}

	{
		std::stringstream stream("pipeline Test {cull_mode = front_and_back;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(CullMode::FrontAndBack, pipelines[0].renderState.rasterizationState.cullMode);
	}

	{
		std::stringstream stream("pipeline Test {cull_mode = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(28U, messages[0].column);
		EXPECT_EQ("invalid cull mode value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, FrontFace)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {front_face = counter_clockwise ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(FrontFace::CounterClockwise,
			pipelines[0].renderState.rasterizationState.frontFace);
	}

	{
		std::stringstream stream("pipeline Test {front_face = clockwise;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(FrontFace::Clockwise, pipelines[0].renderState.rasterizationState.frontFace);
	}

	{
		std::stringstream stream("pipeline Test {front_face = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(29U, messages[0].column);
		EXPECT_EQ("invalid front face value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, DepthBiasEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_bias_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.rasterizationState.depthBiasEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.rasterizationState.depthBiasEnable);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_enable = asdf;}");
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
		EXPECT_EQ("invalid boolean value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, DepthBiasConstantFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_bias_constant_factor = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(-3.0f,
			pipelines[0].renderState.rasterizationState.depthBiasConstantFactor);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_constant_factor = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.3e4f,
			pipelines[0].renderState.rasterizationState.depthBiasConstantFactor);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_constant_factor = asdf;}");
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
		EXPECT_EQ("invalid float value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, DepthBiasClamp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_bias_clamp = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(-3.0f,
			pipelines[0].renderState.rasterizationState.depthBiasClamp);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_clamp = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.3e4f,
			pipelines[0].renderState.rasterizationState.depthBiasClamp);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_clamp = asdf;}");
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
		EXPECT_EQ("invalid float value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, DepthBiasSlopeFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {depth_bias_slope_factor = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(-3.0f,
			pipelines[0].renderState.rasterizationState.depthBiasSlopeFactor);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_slope_factor = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.3e4f,
			pipelines[0].renderState.rasterizationState.depthBiasSlopeFactor);
	}

	{
		std::stringstream stream("pipeline Test {depth_bias_slope_factor = asdf;}");
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
		EXPECT_EQ("invalid float value: 'asdf'", messages[0].message);
	}
}

TEST(ParserRasterizationStateTest, LineWidth)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {line_width = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(-3.0f,
			pipelines[0].renderState.rasterizationState.lineWidth);
	}

	{
		std::stringstream stream("pipeline Test {line_width = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.3e4f,
			pipelines[0].renderState.rasterizationState.lineWidth);
	}

	{
		std::stringstream stream("pipeline Test {line_width = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(29U, messages[0].column);
		EXPECT_EQ("invalid float value: 'asdf'", messages[0].message);
	}
}

} // namespace msl
