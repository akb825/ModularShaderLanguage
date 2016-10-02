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
#include <gtest/gtest.h>

namespace msl
{

TEST(ParserBlendStateTest, LogicalOpEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {logical_op_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.blendState.logicalOpEnable);
	}

	{
		std::stringstream stream("pipeline Test {logical_op_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False, pipelines[0].renderState.blendState.logicalOpEnable);
	}

	{
		std::stringstream stream("pipeline Test {logical_op_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(36U, output.getMessages()[0].column);
		EXPECT_EQ("invalid boolean value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, LogicalOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {logical_op = clear ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Clear, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = and;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::And, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = and_reverse;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::AndReverse, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = copy;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Copy, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = and_inverted;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::AndInverted, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = no_op;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::NoOp, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = xor;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Xor, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = or;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Or, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = nor;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Nor, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = equivalent;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Equivalent, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = invert;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Invert, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = or_reverse;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::OrReverse, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = copy_inverted;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::CopyInverted, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = or_inverted;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::OrInverted, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = nand;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Nand, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = set;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(LogicOp::Set, pipelines[0].renderState.blendState.logicalOp);
	}

	{
		std::stringstream stream("pipeline Test {logical_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(29U, output.getMessages()[0].column);
		EXPECT_EQ("invalid logic op value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, SeparateAttachmentBlendingEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {separate_attachment_blending_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True, pipelines[0].renderState.blendState.separateAttachmentBlendingEnable);
	}

	{
		std::stringstream stream("pipeline Test {separate_attachment_blending_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False,
			pipelines[0].renderState.blendState.separateAttachmentBlendingEnable);
	}

	{
		std::stringstream stream("pipeline Test {separate_attachment_blending_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(54U, output.getMessages()[0].column);
		EXPECT_EQ("invalid boolean value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, BlendEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {blend_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True,
			pipelines[0].renderState.blendState.blendAttachments[0].blendEnable);
	}

	{
		std::stringstream stream("pipeline Test {blend_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False,
			pipelines[0].renderState.blendState.blendAttachments[0].blendEnable);
	}

	{
		std::stringstream stream("pipeline Test {blend_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(31U, output.getMessages()[0].column);
		EXPECT_EQ("invalid boolean value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, SrcColorBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::One,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = src_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::SrcColor,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_src_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrcColor,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = dst_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::DstColor,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_dst_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusDstColor,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::SrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = dst_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::DstAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_dst_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusDstAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = const_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::ConstColor,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_const_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusConstColor,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = const_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::ConstAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_const_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusConstAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = src_alpha_saturate;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::SrcAlphaSaturate,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = src1_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Src1Color,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_src1_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrc1Color,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = src1_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Src1Alpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = one_minus_src1_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrc1Alpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_color_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(41U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, DstColorBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {dst_color_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {dst_color_blend_factor = one_minus_dst_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusDstColor,
			pipelines[0].renderState.blendState.blendAttachments[0].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {dst_color_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(41U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, ColorBlendOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {color_blend_op = add ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {color_blend_op = subtract;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Subtract,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {color_blend_op = reverse_subtract;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {color_blend_op = min;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Min,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {color_blend_op = max;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Max,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {dst_color_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(41U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, SrcAlphaBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {src_alpha_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_alpha_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_alpha_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(41U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, DstAlphaBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {dst_alpha_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {dst_alpha_blend_factor = one_minus_dst_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusDstAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {dst_alpha_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(41U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, AlphaBlendOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {alpha_blend_op = add ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {alpha_blend_op = reverse_subtract;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {alpha_blend_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(33U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend op value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, SrcBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {src_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {src_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(35U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, DstBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {dst_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {dst_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {dst_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(35U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, BlendOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {blend_op = add ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {blend_op = reverse_subtract;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[0].colorBlendOp);
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[0].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {blend_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(27U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend op value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, ColorWriteMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {color_write_mask = 0 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskNone,
			pipelines[0].renderState.blendState.blendAttachments[0].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {color_write_mask = R;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskRed,
			pipelines[0].renderState.blendState.blendAttachments[0].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {color_write_mask = G;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskGreen,
			pipelines[0].renderState.blendState.blendAttachments[0].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {color_write_mask = B;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskBlue,
			pipelines[0].renderState.blendState.blendAttachments[0].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {color_write_mask = A;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {color_write_mask = RGBA;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskRed | ColorMaskGreen | ColorMaskBlue |
			ColorMaskAlpha,
			pipelines[0].renderState.blendState.blendAttachments[0].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {color_write_mask = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(35U, output.getMessages()[0].column);
		EXPECT_EQ("invalid color mask value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, AttachmentBlendEnable)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_blend_enable = true ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::True,
			pipelines[0].renderState.blendState.blendAttachments[3].blendEnable);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_blend_enable = false;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(Bool::False,
			pipelines[0].renderState.blendState.blendAttachments[3].blendEnable);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_blend_enable = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(43U, output.getMessages()[0].column);
		EXPECT_EQ("invalid boolean value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, AttachmentSrcColorBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_src_color_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_src_color_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_src_color_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(53U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, AttachmentDstColorBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_dst_color_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_dst_color_blend_factor = one_minus_dst_color;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusDstColor,
			pipelines[0].renderState.blendState.blendAttachments[3].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_dst_color_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(53U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, AttachmentColorBlendOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_color_blend_op = add ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[3].colorBlendOp);
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_color_blend_op = reverse_subtract;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[3].colorBlendOp);
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_dst_color_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(53U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, AttachmentSrcAlphaBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_src_alpha_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_src_alpha_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_src_alpha_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(53U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, AttachmentDstAlphaBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_dst_alpha_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_dst_alpha_blend_factor = one_minus_dst_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusDstAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_dst_alpha_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(53U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, AttachmentAlphaBlendOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_alpha_blend_op = add ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].colorBlendOp);
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[3].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_alpha_blend_op = reverse_subtract;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Unset,
			pipelines[0].renderState.blendState.blendAttachments[3].colorBlendOp);
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[3].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_alpha_blend_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(45U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend op value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, AttachmentSrcBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_src_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_src_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].srcColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].srcAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_src_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(47U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserBlendStateTest, AttachmentDstBlendFactor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_dst_blend_factor = zero ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::Zero,
			pipelines[0].renderState.blendState.blendAttachments[3].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_dst_blend_factor = one_minus_src_alpha;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].dstColorBlendFactor);
		EXPECT_EQ(BlendFactor::OneMinusSrcAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].dstAlphaBlendFactor);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_dst_blend_factor = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(47U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend factor value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, AttachmentBlendOp)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_blend_op = add ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[3].colorBlendOp);
		EXPECT_EQ(BlendOp::Add,
			pipelines[0].renderState.blendState.blendAttachments[3].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_blend_op = reverse_subtract;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[3].colorBlendOp);
		EXPECT_EQ(BlendOp::ReverseSubtract,
			pipelines[0].renderState.blendState.blendAttachments[3].alphaBlendOp);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_blend_op = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(39U, output.getMessages()[0].column);
		EXPECT_EQ("invalid blend op value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, AttachmentColorWriteMask)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {attachment3_color_write_mask = 0 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskNone,
			pipelines[0].renderState.blendState.blendAttachments[3].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_color_write_mask = RGBA;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(ColorMaskRed | ColorMaskGreen | ColorMaskBlue |
			ColorMaskAlpha,
			pipelines[0].renderState.blendState.blendAttachments[3].colorWriteMask);
	}

	{
		std::stringstream stream("pipeline Test {attachment3_color_write_mask = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(47U, output.getMessages()[0].column);
		EXPECT_EQ("invalid color mask value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParseBlendStateTest, BlendConstants)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {blend_constant = vec4 ( 1.2 ) ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.2f,
			pipelines[0].renderState.blendState.blendConstants[0]);
		EXPECT_EQ(1.2f,
			pipelines[0].renderState.blendState.blendConstants[1]);
		EXPECT_EQ(1.2f,
			pipelines[0].renderState.blendState.blendConstants[2]);
		EXPECT_EQ(1.2f,
			pipelines[0].renderState.blendState.blendConstants[3]);
	}

	{
		std::stringstream stream("pipeline Test {blend_constant = vec4(1.2, -3.4, 5.6, -7.8);}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(1.2f,
			pipelines[0].renderState.blendState.blendConstants[0]);
		EXPECT_EQ(-3.4f,
			pipelines[0].renderState.blendState.blendConstants[1]);
		EXPECT_EQ(5.6f,
			pipelines[0].renderState.blendState.blendConstants[2]);
		EXPECT_EQ(-7.8f,
			pipelines[0].renderState.blendState.blendConstants[3]);
	}

	{
		std::stringstream stream("pipeline Test {blend_constant = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(33U, output.getMessages()[0].column);
		EXPECT_EQ("invalid vec4 value: asdf", output.getMessages()[0].message);
	}

	{
		std::stringstream stream("pipeline Test {blend_constant = vec3(1.2, 3.4, 5.6);}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(33U, output.getMessages()[0].column);
		EXPECT_EQ("invalid vec4 value: vec3(1.2,3.4,5.6)", output.getMessages()[0].message);
	}

	{
		std::stringstream stream("pipeline Test {blend_constant = vec4(1.2, 3.4, 5.6);}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(33U, output.getMessages()[0].column);
		EXPECT_EQ("invalid vec4 value: vec4(1.2,3.4,5.6)", output.getMessages()[0].message);
	}

	{
		std::stringstream stream("pipeline Test {blend_constant = vec4(1.2, 3.4, 5.6, 7.8, 9.0);}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(33U, output.getMessages()[0].column);
		EXPECT_EQ("invalid vec4 value: vec4(1.2,3.4,5.6,7.8,9.0)", output.getMessages()[0].message);
	}

	{
		std::stringstream stream("pipeline Test {blend_constant = vec4(1.2, 3.4, 5.6, 7.8;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(33U, output.getMessages()[0].column);
		EXPECT_EQ("invalid vec4 value: vec4(1.2,3.4,5.6,7.8", output.getMessages()[0].message);
	}

	{
		std::stringstream stream("pipeline Test {blend_constant = vec4(1.2, 3.4, 5.6, asdf);}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(33U, output.getMessages()[0].column);
		EXPECT_EQ("invalid float value: asdf", output.getMessages()[0].message);
	}
}

} // namespace msl
