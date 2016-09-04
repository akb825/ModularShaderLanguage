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

TEST(ParserSamplerTest, MinFilter)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {min_filter = nearest ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::Filter::Nearest, samplers[0].minFilter);
	}

	{
		std::stringstream stream("sampler_state Test {min_filter = linear;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::Filter::Linear, samplers[0].minFilter);
	}

	{
		std::stringstream stream("sampler_state Test {min_filter = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(34U, output.getMessages()[0].column);
		EXPECT_EQ("invalid filter value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, MagFilter)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {mag_filter = nearest ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::Filter::Nearest, samplers[0].magFilter);
	}

	{
		std::stringstream stream("sampler_state Test {mag_filter = linear;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::Filter::Linear, samplers[0].magFilter);
	}

	{
		std::stringstream stream("sampler_state Test {mag_filter = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(34U, output.getMessages()[0].column);
		EXPECT_EQ("invalid filter value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, MipFilter)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {mip_filter = none ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::MipFilter::None, samplers[0].mipFilter);
	}

	{
		std::stringstream stream("sampler_state Test {mip_filter = nearest;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::MipFilter::Nearest, samplers[0].mipFilter);
	}

	{
		std::stringstream stream("sampler_state Test {mip_filter = linear;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::MipFilter::Linear, samplers[0].mipFilter);
	}

	{
		std::stringstream stream("sampler_state Test {mip_filter = anisotropic;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::MipFilter::Anisotropic, samplers[0].mipFilter);
	}

	{
		std::stringstream stream("sampler_state Test {mip_filter = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(34U, output.getMessages()[0].column);
		EXPECT_EQ("invalid mip filter value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, AddressModeU)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {address_mode_u = repeat ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::Repeat, samplers[0].addressModeU);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_u = mirrored_repeat;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::MirroredRepeat, samplers[0].addressModeU);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_u = clamp_to_edge;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::ClampToEdge, samplers[0].addressModeU);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_u = clamp_to_border;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::ClampToBorder, samplers[0].addressModeU);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_u = mirror_once;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::MirrorOnce, samplers[0].addressModeU);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_u = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(38U, output.getMessages()[0].column);
		EXPECT_EQ("invalid address mode value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, AddressModeV)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {address_mode_v = repeat ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::Repeat, samplers[0].addressModeV);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_v = clamp_to_edge;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::ClampToEdge, samplers[0].addressModeV);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_v = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(38U, output.getMessages()[0].column);
		EXPECT_EQ("invalid address mode value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, AddressModeW)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {address_mode_w = repeat ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::Repeat, samplers[0].addressModeW);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_w = clamp_to_edge;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::AddressMode::ClampToEdge, samplers[0].addressModeW);
	}

	{
		std::stringstream stream("sampler_state Test {address_mode_w = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(38U, output.getMessages()[0].column);
		EXPECT_EQ("invalid address mode value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, MipLodBias)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {mip_lod_bias = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(-3.0f, samplers[0].mipLodBias);
	}

	{
		std::stringstream stream("sampler_state Test {mip_lod_bias = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(1.3e4, samplers[0].mipLodBias);
	}

	{
		std::stringstream stream("sampler_state Test {mip_lod_bias = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(36U, output.getMessages()[0].column);
		EXPECT_EQ("invalid float value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, MaxAnisotropy)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {max_anisotropy = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(-3.0f, samplers[0].maxAnisotropy);
	}

	{
		std::stringstream stream("sampler_state Test {max_anisotropy = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(1.3e4, samplers[0].maxAnisotropy);
	}

	{
		std::stringstream stream("sampler_state Test {max_anisotropy = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(38U, output.getMessages()[0].column);
		EXPECT_EQ("invalid float value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, MinLod)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {min_lod = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(-3.0f, samplers[0].minLod);
	}

	{
		std::stringstream stream("sampler_state Test {min_lod = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(1.3e4, samplers[0].minLod);
	}

	{
		std::stringstream stream("sampler_state Test {min_lod = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(31U, output.getMessages()[0].column);
		EXPECT_EQ("invalid float value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, MaxLod)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {max_lod = - 3 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(-3.0f, samplers[0].maxLod);
	}

	{
		std::stringstream stream("sampler_state Test {max_lod = 1.3e4;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(1.3e4, samplers[0].maxLod);
	}

	{
		std::stringstream stream("sampler_state Test {max_lod = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(31U, output.getMessages()[0].column);
		EXPECT_EQ("invalid float value: asdf", output.getMessages()[0].message);
	}
}

TEST(ParserSamplerTest, BorderColor)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("sampler_state Test {border_color = transparent_black ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::BorderColor::TransparentBlack, samplers[0].borderColor);
	}

	{
		std::stringstream stream("sampler_state Test {border_color = transparent_int_zero;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::BorderColor::TransparentIntZero, samplers[0].borderColor);
	}

	{
		std::stringstream stream("sampler_state Test {border_color = opaque_black;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::BorderColor::OpaqueBlack, samplers[0].borderColor);
	}

	{
		std::stringstream stream("sampler_state Test {border_color = opaque_int_zero;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::BorderColor::OpaqueIntZero, samplers[0].borderColor);
	}

	{
		std::stringstream stream("sampler_state Test {border_color = opaque_white;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::BorderColor::OpaqueWhite, samplers[0].borderColor);
	}

	{
		std::stringstream stream("sampler_state Test {border_color = opaque_int_one;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Sampler>& samplers = parser.getSamplers();
		ASSERT_EQ(1U, samplers.size());
		EXPECT_EQ(Parser::BorderColor::OpaqueIntOne, samplers[0].borderColor);
	}

	{
		std::stringstream stream("sampler_state Test {border_color = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		ASSERT_EQ(1U, output.getMessages().size());
		EXPECT_EQ(path, output.getMessages()[0].file);
		EXPECT_EQ(1U, output.getMessages()[0].line);
		EXPECT_EQ(36U, output.getMessages()[0].column);
		EXPECT_EQ("invalid border color value: asdf", output.getMessages()[0].message);
	}
}

} // namespace msl
