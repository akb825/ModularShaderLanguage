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

#include "Parser.h"
#include <MSL/Compile/Output.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <cstring>
#include <unordered_set>

namespace msl
{

static std::unordered_set<std::string> opaqueTypes =
{
	// Samplers
	{"sampler1D"},
	{"sampler2D"},
	{"sampler3D"},
	{"samplerCube"},
	{"sampler1DShadow"},
	{"sampler2DShadow"},
	{"sampler1DArray"},
	{"sampler2DArray"},
	{"sampler1DArrayShadow"},
	{"sampler2DArrayShadow"},
	{"sampler2DMS"},
	{"sampler2DMSArray"},
	{"samplerCubeShadow"},
	{"samplerBuffer"},
	{"sampler2DRect"},
	{"sampler2DRectShadow"},
	{"isampler1D"},
	{"isampler2D"},
	{"isampler3D"},
	{"isamplerCube"},
	{"isampler1DArray"},
	{"isampler2DArray"},
	{"isampler2DMS"},
	{"isampler2DMSArray"},
	{"isampler2DRect"},
	{"usampler1D"},
	{"usampler2D"},
	{"usampler3D"},
	{"usamplerCube"},
	{"usampler1DArray"},
	{"usampler2DArray"},
	{"usampler2DMS"},
	{"usampler2DMSArray"},
	{"usampler2DRect"},

	// Images
	{"image1D"},
	{"image2D"},
	{"image3D"},
	{"imageCube"},
	{"image1DArray"},
	{"image2DArray"},
	{"image2DMS"},
	{"image2DMSArray"},
	{"imageBuffer"},
	{"image2DRect"},
	{"iimage1D"},
	{"iimage2D"},
	{"iimage3D"},
	{"iimageCube"},
	{"iimage1DArray"},
	{"iimage2DArray"},
	{"iimage2DMS"},
	{"iimage2DMSArray"},
	{"iimage2DRect"},
	{"uimage1D"},
	{"uimage2D"},
	{"uimage3D"},
	{"uimageCube"},
	{"uimage1DArray"},
	{"uimage2DArray"},
	{"uimage2DMS"},
	{"uimage2DMSArray"},
	{"uimage2DRect"},

	// Subpass inputs
	{"subpassInput"},
	{"subpassInputMS"},
	{"isubpassInput"},
	{"isubpassInputMS"},
	{"usubpassInput"},
	{"usubpassInputMS"}
};

static bool skipWhitespace(const std::vector<Token>& tokens, std::size_t& i, std::size_t maxValue)
{
	for (; i < maxValue; ++i)
	{
		if (tokens[i].type != Token::Type::Whitespace)
			return true;
	}

	return false;
}

static bool skipWhitespace(Output& output, const std::vector<Token>& tokens, std::size_t& i)
{
	if (skipWhitespace(tokens, i, tokens.size()))
		return true;

	const Token& lastToken = tokens.back();
	output.addMessage(Output::Level::Error, lastToken .fileName, lastToken .line,
		lastToken .column, false, "unexpected end of file");
	return false;
}

enum class KeyValueResult
{
	Success,
	Error,
	End
};

KeyValueResult readKeyValue(Output& output, const Token*& key, const Token*& value,
	const std::vector<Token>& tokens, std::size_t& i)
{
	do
	{
		// Read the contents of the block
		if (!skipWhitespace(output, tokens, i))
			return KeyValueResult::Error;

		if (tokens[i].value == ";")
		{
			// Empty ;
			++i;
			continue;
		}
		else if (tokens[i].value == "}")
		{
			// End of declaration block
			++i;
			return KeyValueResult::End;
		}
		else
		{
			// key = value;
			if (tokens[i].type != Token::Type::Identifier)
			{
				output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
					tokens[i].column, false, "unexpected token: " + tokens[i].value);
				return KeyValueResult::Error;
			}

			key = &tokens[i];

			if (!skipWhitespace(output, tokens, ++i))
				return KeyValueResult::Error;

			if (tokens[i].value != "=")
			{
				output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
					tokens[i].column, false, "unexpected token: " + tokens[i].value);
				return KeyValueResult::Error;
			}

			if (!skipWhitespace(output, tokens, ++i))
				return KeyValueResult::Error;

			if (tokens[i].type == Token::Type::Symbol)
			{
				output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
					tokens[i].column, false, "unexpected token: " + tokens[i].value);
				return KeyValueResult::Error;
			}

			value = &tokens[i];

			if (!skipWhitespace(output, tokens, ++i))
				return KeyValueResult::Error;

			if (tokens[i].value != ";")
			{
				output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
					tokens[i].column, false, "unexpected token: " + tokens[i].value);
				return KeyValueResult::Error;
			}

			++i;
			return KeyValueResult::Success;
		}
	} while (i < tokens.size());

	const Token& lastToken = tokens.back();
	output.addMessage(Output::Level::Error, lastToken.fileName, lastToken.line,
		lastToken.column, false,
		"unexpected end of file");
	return KeyValueResult::Error;
}

static bool getStage(Parser::Stage& stage, const Token& token)
{
	if (token.value == "vertex")
	{
		stage = Parser::Stage::Vertex;
		return true;
	}
	else if (token.value == "tessellation_control")
	{
		stage = Parser::Stage::TessellationControl;
		return true;
	}
	else if (token.value == "tessellation_evaluation")
	{
		stage = Parser::Stage::TessellationEvaluation;
		return true;
	}
	else if (token.value == "geometry")
	{
		stage = Parser::Stage::Geometry;
		return true;
	}
	else if (token.value == "fragment")
	{
		stage = Parser::Stage::Fragment;
		return true;
	}
	else if (token.value == "compute")
	{
		stage = Parser::Stage::Compute;
		return true;
	}

	return false;
}

static bool getBool(Output& output, Parser::Bool& value, const Token& token)
{
	if (token.value == "true")
	{
		value = Parser::Bool::True;
		return true;
	}
	else if (token.value == "false")
	{
		value = Parser::Bool::False;
		return true;
	}
	else
	{
		try
		{
			value = boost::lexical_cast<bool>(token.value) ?
				Parser::Bool::True : Parser::Bool::False;
			return true;
		}
		catch (...)
		{
			output.addMessage(Output::Level::Error, token.fileName, token.line,
				token.column, false, "invalid boolean value: " + token.value);
			return false;
		}
	}
}

static bool getInt(Output& output, std::uint32_t& value, const Token& token)
{
	try
	{
		value = boost::lexical_cast<std::uint32_t>(token.value);
		return true;
	}
	catch (...)
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid int value: " + token.value);
		return false;
	}
}

static bool getFloat(Output& output, float& value, const Token& token)
{
	try
	{
		value = boost::lexical_cast<float>(token.value);
		return true;
	}
	catch (...)
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid float value: " + token.value);
		return false;
	}
}

static bool getPolygonMode(Output& output, Parser::PolygonMode& value, const Token& token)
{
	if (token.value == "fill")
	{
		value = Parser::PolygonMode::Fill;
		return true;
	}
	else if (token.value == "line")
	{
		value = Parser::PolygonMode::Line;
		return true;
	}
	else if (token.value == "Point")
	{
		value = Parser::PolygonMode::Point;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid polygon mode value: " + token.value);
		return false;
	}
}

static bool getCullMode(Output& output, Parser::CullMode& value, const Token& token)
{
	if (token.value == "none")
	{
		value = Parser::CullMode::None;
		return true;
	}
	else if (token.value == "front")
	{
		value = Parser::CullMode::Front;
		return true;
	}
	else if (token.value == "back")
	{
		value = Parser::CullMode::Back;
		return true;
	}
	else if (token.value == "front_and_back")
	{
		value = Parser::CullMode::FrontAndBack;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid cull mode value: " + token.value);
		return false;
	}
}

static bool getFrontFace(Output& output, Parser::FrontFace& value, const Token& token)
{
	if (token.value == "counter_clockwise")
	{
		value = Parser::FrontFace::CounterClockwise;
		return true;
	}
	else if (token.value == "clockwise")
	{
		value = Parser::FrontFace::Clockwise;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid front face value: " + token.value);
		return false;
	}
}

static bool getCompareOp(Output& output, Parser::CompareOp& value, const Token& token)
{
	if (token.value == "never")
	{
		value = Parser::CompareOp::Never;
		return true;
	}
	else if (token.value == "less")
	{
		value = Parser::CompareOp::Less;
		return true;
	}
	else if (token.value == "equal")
	{
		value = Parser::CompareOp::Equal;
		return true;
	}
	else if (token.value == "less_or_equal")
	{
		value = Parser::CompareOp::LessOrEqual;
		return true;
	}
	else if (token.value == "greater")
	{
		value = Parser::CompareOp::Greater;
		return true;
	}
	else if (token.value == "not_equal")
	{
		value = Parser::CompareOp::NotEqual;
		return true;
	}
	else if (token.value == "greater_or_equal")
	{
		value = Parser::CompareOp::GreaterOrEqual;
		return true;
	}
	else if (token.value == "always")
	{
		value = Parser::CompareOp::Always;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid compare op value: " + token.value);
		return false;
	}
}

static bool getBlendFactor(Output& output, Parser::BlendFactor& value, const Token& token)
{
	if (token.value == "zero")
	{
		value = Parser::BlendFactor::Zero;
		return true;
	}
	else if (token.value == "one")
	{
		value = Parser::BlendFactor::One;
		return true;
	}
	else if (token.value == "src_color")
	{
		value = Parser::BlendFactor::SrcColor;
		return true;
	}
	else if (token.value == "one_minus_src_color")
	{
		value = Parser::BlendFactor::OneMinusSrcColor;
		return true;
	}
	else if (token.value == "dst_color")
	{
		value = Parser::BlendFactor::DstColor;
		return true;
	}
	else if (token.value == "one_minus_dst_color")
	{
		value = Parser::BlendFactor::OneMinusDstColor;
		return true;
	}
	else if (token.value == "src_alpha")
	{
		value = Parser::BlendFactor::SrcAlpha;
		return true;
	}
	else if (token.value == "one_minus_src_alpha")
	{
		value = Parser::BlendFactor::OneMinusSrcAlpha;
		return true;
	}
	else if (token.value == "dst_alpha")
	{
		value = Parser::BlendFactor::DstAlpha;
		return true;
	}
	else if (token.value == "one_minus_dst_alpha")
	{
		value = Parser::BlendFactor::OneMinusDstAlpha;
		return true;
	}
	else if (token.value == "const_color")
	{
		value = Parser::BlendFactor::ConstColor;
		return true;
	}
	else if (token.value == "one_minus_const_color")
	{
		value = Parser::BlendFactor::OneMinusConstColor;
		return true;
	}
	else if (token.value == "const_alpha")
	{
		value = Parser::BlendFactor::ConstAlpha;
		return true;
	}
	else if (token.value == "one_minus_const_alpha")
	{
		value = Parser::BlendFactor::OneMinusConstAlpha;
		return true;
	}
	else if (token.value == "src_alpha_saturate")
	{
		value = Parser::BlendFactor::SrcAlphaSaturate;
		return true;
	}
	else if (token.value == "src1_color")
	{
		value = Parser::BlendFactor::Src1Color;
		return true;
	}
	else if (token.value == "one_minus_src1_color")
	{
		value = Parser::BlendFactor::OneMinusSrc1Color;
		return true;
	}
	else if (token.value == "src1_alpha")
	{
		value = Parser::BlendFactor::Src1Alpha;
		return true;
	}
	else if (token.value == "one_minus_src1_alpha")
	{
		value = Parser::BlendFactor::OneMinusSrc1Alpha;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid blend factor value: " + token.value);
		return false;
	}
}

static bool getBlendOp(Output& output, Parser::BlendOp& value, const Token& token)
{
	if (token.value == "add")
	{
		value = Parser::BlendOp::Add;
		return true;
	}
	else if (token.value == "subtract")
	{
		value = Parser::BlendOp::Subtract;
		return true;
	}
	else if (token.value == "reverse_subtract")
	{
		value = Parser::BlendOp::ReverseSubtract;
		return true;
	}
	else if (token.value == "min")
	{
		value = Parser::BlendOp::Min;
		return true;
	}
	else if (token.value == "max")
	{
		value = Parser::BlendOp::Max;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid blend op value: " + token.value);
		return false;
	}
}

static bool getColorMask(Output& output, Parser::ColorMask& value, const Token& token)
{
	if (token.value == "0")
	{
		value = Parser::ColorMaskNone;
		return true;
	}

	value = Parser::ColorMaskNone;
	for (std::size_t i = 0; i < token.value.size(); ++i)
	{
		switch (token.value[i])
		{
			case 'r':
			case 'R':
				value = static_cast<Parser::ColorMask>(value | Parser::ColorMaskRed);
				break;
			case 'g':
			case 'G':
				value = static_cast<Parser::ColorMask>(value | Parser::ColorMaskGreen);
				break;
			case 'b':
			case 'B':
				value = static_cast<Parser::ColorMask>(value | Parser::ColorMaskBlue);
				break;
			case 'a':
			case 'A':
				value = static_cast<Parser::ColorMask>(value | Parser::ColorMaskAlpha);
				break;
			default:
				output.addMessage(Output::Level::Error, token.fileName, token.line,
					token.column, false, "invalid color mask value: " + token.value);
				return false;
		}
	}

	return true;
}

static bool getStencilOp(Output& output, Parser::StencilOp& value, const Token& token)
{
	if (token.value == "keep")
	{
		value = Parser::StencilOp::Keep;
		return true;
	}
	else if (token.value == "zero")
	{
		value = Parser::StencilOp::Zero;
		return true;
	}
	else if (token.value == "replace")
	{
		value = Parser::StencilOp::Replace;
		return true;
	}
	else if (token.value == "increment_and_clamp")
	{
		value = Parser::StencilOp::IncrementAndClamp;
		return true;
	}
	else if (token.value == "decrement_and_clamp")
	{
		value = Parser::StencilOp::DecrementAndClamp;
		return true;
	}
	else if (token.value == "invert")
	{
		value = Parser::StencilOp::Invert;
		return true;
	}
	else if (token.value == "increment_and_wrap")
	{
		value = Parser::StencilOp::IncrementAndWrap;
		return true;
	}
	else if (token.value == "decrement_and_wrap")
	{
		value = Parser::StencilOp::DecrementAndWrap;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid stencil op value: " + token.value);
		return false;
	}
}

static bool getLogicalOp(Output& output, Parser::LogicOp& value, const Token& token)
{
	if (token.value == "clear")
	{
		value = Parser::LogicOp::Clear;
		return true;
	}
	else if (token.value == "and")
	{
		value = Parser::LogicOp::And;
		return true;
	}
	else if (token.value == "and_reverse")
	{
		value = Parser::LogicOp::AndReverse;
		return true;
	}
	else if (token.value == "copy")
	{
		value = Parser::LogicOp::Copy;
		return true;
	}
	else if (token.value == "and_inverted")
	{
		value = Parser::LogicOp::AndInverted;
		return true;
	}
	else if (token.value == "no_op")
	{
		value = Parser::LogicOp::NoOp;
		return true;
	}
	else if (token.value == "xor")
	{
		value = Parser::LogicOp::Xor;
		return true;
	}
	else if (token.value == "or")
	{
		value = Parser::LogicOp::Or;
		return true;
	}
	else if (token.value == "nor")
	{
		value = Parser::LogicOp::Nor;
		return true;
	}
	else if (token.value == "or_reverse")
	{
		value = Parser::LogicOp::OrReverse;
		return true;
	}
	else if (token.value == "copy_inverted")
	{
		value = Parser::LogicOp::CopyInverted;
		return true;
	}
	else if (token.value == "or_inverted")
	{
		value = Parser::LogicOp::OrInverted;
		return true;
	}
	else if (token.value == "nand")
	{
		value = Parser::LogicOp::Nand;
		return true;
	}
	else if (token.value == "set")
	{
		value = Parser::LogicOp::Set;
		return true;
	}
	else
	{
		output.addMessage(Output::Level::Error, token.fileName, token.line,
			token.column, false, "invalid logical op value: " + token.value);
		return false;
	}
}

static bool isAttachment(unsigned int& index, const std::string& key, const char* field)
{
	const char* attachmentStr = "attachment";
	if (boost::algorithm::starts_with(key, attachmentStr))
	{
		std::size_t attachmentStrLen = std::strlen(attachmentStr);
		std::size_t separatorIdx = key.find_first_of('_', attachmentStrLen);
		if (separatorIdx == std::string::npos)
			return false;

		std::string indexStr = key.substr(attachmentStrLen, separatorIdx - attachmentStrLen);
		try
		{
			index = boost::lexical_cast<unsigned int>(indexStr);
			if (index >= Parser::maxAttachments)
				return false;
		}
		catch (...)
		{
			return false;
		}

		return key.compare(separatorIdx + 1, std::strlen(field), field) == 0;
	}
	else
	{
		index = 0;
		return key == field;
	}
}

enum class ParseResult
{
	Success,
	NotThisType,
	Error
};

static ParseResult readStage(Output& output, Parser::Pipeline& pipeline, const Token& key,
	const Token& value)
{
	Parser::Stage stage;
	if (!getStage(stage, key))
		return ParseResult::NotThisType;

	if (value.type != Token::Type::Identifier)
	{
		output.addMessage(Output::Level::Error, value.fileName, value.line,
			value.column, false, "unexpected token: " + value.value);
		return ParseResult::Error;
	}

	pipeline.entryPoints[static_cast<int>(stage)] = value.value;
	return ParseResult::Success;
}

static ParseResult readRenderState(Output& output, Parser::Pipeline& pipeline, const Token& key,
	const Token& value)
{
	unsigned int attachmentIndex;

	// RasterizationState
	if (key.value == "depth_clamp_enable")
	{
		if (!getBool(output, pipeline.renderState.rasterizationState.depthClampEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "rasterizer_discard_enable")
	{
		if (!getBool(output, pipeline.renderState.rasterizationState.rasterizerDiscardEnable,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "polygon_mode")
	{
		if (!getPolygonMode(output, pipeline.renderState.rasterizationState.polygonMode, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "cull_mode")
	{
		if (!getCullMode(output, pipeline.renderState.rasterizationState.cullMode, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "front_face")
	{
		if (!getFrontFace(output, pipeline.renderState.rasterizationState.frontFace, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "depth_bias_enable")
	{
		if (!getBool(output, pipeline.renderState.rasterizationState.depthBiasEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "depth_bias_constant_factor")
	{
		if (!getFloat(output, pipeline.renderState.rasterizationState.depthBiasConstantFactor,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "depth_bias_clamp")
	{
		if (!getFloat(output, pipeline.renderState.rasterizationState.depthBiasClamp, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "depth_bias_slope_factor")
	{
		if (!getFloat(output, pipeline.renderState.rasterizationState.depthBiasSlopeFactor, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "line_width")
	{
		if (!getFloat(output, pipeline.renderState.rasterizationState.lineWidth, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	// MultisampleState
	else if (key.value == "sample_shading_enable")
	{
		if (!getBool(output, pipeline.renderState.multisampleState.sampleShadingEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "sample_mask")
	{
		if (!getInt(output, pipeline.renderState.multisampleState.sampleMask, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "alpha_to_coverage_enable")
	{
		if (!getBool(output, pipeline.renderState.multisampleState.alphaToCoverageEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "alpha_to_one_enable")
	{
		if (!getBool(output, pipeline.renderState.multisampleState.alphaToOneEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	// DepthStencilState
	else if (key.value == "depth_test_enable")
	{
		if (!getBool(output, pipeline.renderState.depthStencilState.depthTestEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "depth_write_enable")
	{
		if (!getBool(output, pipeline.renderState.depthStencilState.depthWriteEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "depth_compare_op")
	{
		if (!getCompareOp(output, pipeline.renderState.depthStencilState.depthCompareOp, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "depth_bounds_test_enable")
	{
		if (!getBool(output, pipeline.renderState.depthStencilState.depthBoundsTestEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "stencil_test_enable")
	{
		if (!getBool(output, pipeline.renderState.depthStencilState.stencilTestEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "min_depth_bounds")
	{
		if (!getFloat(output, pipeline.renderState.depthStencilState.minDepthBounds, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "max_depth_bounds")
	{
		if (!getFloat(output, pipeline.renderState.depthStencilState.maxDepthBounds, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	// Combined front/back stencil states
	else if (key.value == "stencil_fail_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.frontStencil.failOp,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.depthStencilState.backStencil.failOp =
			pipeline.renderState.depthStencilState.frontStencil.failOp;
		return ParseResult::Success;
	}
	else if (key.value == "stencil_pass_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.frontStencil.passOp,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.depthStencilState.backStencil.passOp =
			pipeline.renderState.depthStencilState.frontStencil.passOp;
		return ParseResult::Success;
	}
	else if (key.value == "stencil_depth_fail_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.frontStencil.depthFailOp,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.depthStencilState.backStencil.depthFailOp =
			pipeline.renderState.depthStencilState.frontStencil.depthFailOp;
		return ParseResult::Success;
	}
	else if (key.value == "stencil_compare_op")
	{
		if (!getCompareOp(output, pipeline.renderState.depthStencilState.frontStencil.compareOp,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.depthStencilState.backStencil.compareOp =
			pipeline.renderState.depthStencilState.frontStencil.compareOp;
		return ParseResult::Success;
	}
	else if (key.value == "stencil_compare_op")
	{
		if (!getCompareOp(output, pipeline.renderState.depthStencilState.frontStencil.compareOp,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.depthStencilState.backStencil.compareOp =
			pipeline.renderState.depthStencilState.frontStencil.compareOp;
		return ParseResult::Success;
	}
	else if (key.value == "stencil_write_mask")
	{
		if (!getInt(output, pipeline.renderState.depthStencilState.frontStencil.writeMask, value))
			return ParseResult::Error;
		pipeline.renderState.depthStencilState.backStencil.writeMask =
			pipeline.renderState.depthStencilState.frontStencil.writeMask;
		return ParseResult::Success;
	}
	else if (key.value == "stencil_reference")
	{
		if (!getInt(output, pipeline.renderState.depthStencilState.frontStencil.reference, value))
			return ParseResult::Error;
		pipeline.renderState.depthStencilState.backStencil.reference =
			pipeline.renderState.depthStencilState.frontStencil.reference;
		return ParseResult::Success;
	}
	// Front stencil states
	else if (key.value == "front_stencil_fail_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.frontStencil.failOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "front_stencil_pass_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.frontStencil.passOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "front_stencil_depth_fail_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.frontStencil.depthFailOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "front_stencil_compare_op")
	{
		if (!getCompareOp(output, pipeline.renderState.depthStencilState.frontStencil.compareOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "front_stencil_compare_op")
	{
		if (!getCompareOp(output, pipeline.renderState.depthStencilState.frontStencil.compareOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "front_stencil_write_mask")
	{
		if (!getInt(output, pipeline.renderState.depthStencilState.frontStencil.writeMask, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "front_stencil_reference")
	{
		if (!getInt(output, pipeline.renderState.depthStencilState.frontStencil.reference, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	// Back stencil states
	else if (key.value == "back_stencil_fail_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.backStencil.failOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "back_stencil_pass_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.backStencil.passOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "back_stencil_depth_fail_op")
	{
		if (!getStencilOp(output, pipeline.renderState.depthStencilState.backStencil.depthFailOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "back_stencil_compare_op")
	{
		if (!getCompareOp(output, pipeline.renderState.depthStencilState.backStencil.compareOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "back_stencil_compare_op")
	{
		if (!getCompareOp(output, pipeline.renderState.depthStencilState.backStencil.compareOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (key.value == "back_stencil_write_mask")
	{
		if (!getInt(output, pipeline.renderState.depthStencilState.backStencil.writeMask, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "back_stencil_reference")
	{
		if (!getInt(output, pipeline.renderState.depthStencilState.backStencil.reference, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	// BlendState
	else if (key.value == "logical_op_enable")
	{
		if (!getBool(output, pipeline.renderState.blendState.logicalOpEnable, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "logical_op")
	{
		if (!getLogicalOp(output, pipeline.renderState.blendState.logicalOp, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "separate_attachment_blending")
	{
		if (!getBool(output, pipeline.renderState.blendState.separateAttachmentBlending, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "blend_constant_r")
	{
		if (!getFloat(output, pipeline.renderState.blendState.blendConstants[0], value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "blend_constant_g")
	{
		if (!getFloat(output, pipeline.renderState.blendState.blendConstants[1], value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "blend_constant_b")
	{
		if (!getFloat(output, pipeline.renderState.blendState.blendConstants[2], value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else if (key.value == "blend_constant_a")
	{
		if (!getFloat(output, pipeline.renderState.blendState.blendConstants[3], value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	// BlendAttachmentState
	else if (isAttachment(attachmentIndex, key.value, "blend_enable"))
	{
		if (!getBool(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].blendEnable, value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (isAttachment(attachmentIndex, key.value, "color_write_mask"))
	{
		if (!getColorMask(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].colorWriteMask,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	// Combined blend states
	else if (isAttachment(attachmentIndex, key.value, "src_blend_factor"))
	{
		if (!getBlendFactor(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].srcColorBlendFactor,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.blendState.blendAttachments[attachmentIndex].srcAlphaBlendFactor =
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].srcColorBlendFactor;
		return ParseResult::Success;
	}
	else if (isAttachment(attachmentIndex, key.value, "dst_blend_factor"))
	{
		if (!getBlendFactor(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].dstColorBlendFactor,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.blendState.blendAttachments[attachmentIndex].dstAlphaBlendFactor =
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].dstColorBlendFactor;
		return ParseResult::Success;
	}
	else if (isAttachment(attachmentIndex, key.value, "blend_op"))
	{
		if (!getBlendOp(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].colorBlendOp,
			value))
		{
			return ParseResult::Error;
		}
		pipeline.renderState.blendState.blendAttachments[attachmentIndex].alphaBlendOp =
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].colorBlendOp;
		return ParseResult::Success;
	}
	// Color blend states
	else if (isAttachment(attachmentIndex, key.value, "src_color_blend_factor"))
	{
		if (!getBlendFactor(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].srcColorBlendFactor,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (isAttachment(attachmentIndex, key.value, "dst_color_blend_factor"))
	{
		if (!getBlendFactor(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].dstColorBlendFactor,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (isAttachment(attachmentIndex, key.value, "color_blend_op"))
	{
		if (!getBlendOp(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].colorBlendOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	// Alpha blend states
	else if (isAttachment(attachmentIndex, key.value, "src_alpha_blend_factor"))
	{
		if (!getBlendFactor(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].srcAlphaBlendFactor,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (isAttachment(attachmentIndex, key.value, "dst_alpha_blend_factor"))
	{
		if (!getBlendFactor(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].dstAlphaBlendFactor,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	else if (isAttachment(attachmentIndex, key.value, "alpha_blend_op"))
	{
		if (!getBlendOp(output,
			pipeline.renderState.blendState.blendAttachments[attachmentIndex].alphaBlendOp,
			value))
		{
			return ParseResult::Error;
		}
		return ParseResult::Success;
	}
	// Patch control points
	else if (isAttachment(attachmentIndex, key.value, "patch_control_points"))
	{
		if (!getInt(output, pipeline.renderState.patchControlPoints, value))
			return ParseResult::Error;
		return ParseResult::Success;
	}
	else
		return ParseResult::NotThisType;
}

bool Parser::parse(Output& output, int options)
{
	enum class Element
	{
		Unknown,
		Uniform,
		Buffer,
		Struct,
		In,
		Out
	};

	m_options = options;
	for (auto& elementType : m_elements)
	{
		for (auto& elements : elementType)
			elements.clear();
	}
	m_pipelines.clear();
	
	unsigned int parenCount = 0;
	unsigned int braceCount = 0;
	unsigned int squareCount = 0;
	bool elementStart = true;
	bool inStageDecl = false;
	bool hadScope = false;
	Element element = Element::Unknown;

	const Token* lastToken = nullptr;
	const Token* startParenToken = nullptr;
	const Token* startBraceToken = nullptr;
	const Token* startSquareToken = nullptr;

	TokenRange tokenRange = {};
	std::vector<Stage> stages;
	const auto& tokens = m_tokens.getTokens();
	if (tokens.empty())
		return true;

	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		const Token& token = tokens[i];
		// Skip whitespace, not setting the last token.
		if (token.type == Token::Type::Whitespace)
			continue;

		// Handle stage declarations
		if (inStageDecl)
		{
			if (squareCount != 2 || token.type == Token::Type::Symbol)
			{
				if (token.value == "]")
				{
					// fall through to handle end square brace
				}
				else if (token.value != "," || (lastToken && lastToken->value == ","))
				{
					output.addMessage(Output::Level::Error, token.fileName, token.line,
						token.column, false, "unexpected token: " + token.value);
					return false;
				}
			}
			else
			{
				Stage stage;
				if (!getStage(stage, token))
				{
					output.addMessage(Output::Level::Error, token.fileName, token.line,
						token.column, false, "unknown stage type: " + token.value);
					return false;
				}

				stages.push_back(stage);
				lastToken = &token;
				continue;
			}
		}

		// Declarations that must be in the start: pipeline and [ for stage declaration.
		if (elementStart && token.value == "pipeline")
		{
			if (!readPipeline(output, tokens, ++i))
				return false;

			if (i >= tokens.size())
				break;
			lastToken = &tokens[i];
		}
		else if (token.value == "[")
		{
			if (squareCount == 0)
				startSquareToken = &token;
			++squareCount;

			// Handle begin of stage declaration.
			if (squareCount == 2 && lastToken && lastToken->value == "[")
			{
				if (elementStart)
					inStageDecl = true;
				else
				{
					output.addMessage(Output::Level::Error, token.fileName, token.line, token.column,
						false, "stage declaration must be at the start of an element");
					return false;
				}
			}
		}
		else
		{
			elementStart = false;
			if (token.value == "(")
			{
				if (parenCount == 0)
					startParenToken = &token;
				++parenCount;
			}
			else if (token.value == ")")
			{
				if (parenCount == 0)
				{
					output.addMessage(Output::Level::Error, token.fileName, token.line,
						token.column, false, "encountered ) without opening (");
					return false;
				}
				--parenCount;
			}
			if (token.value == "{")
			{
				if (braceCount == 0)
					startBraceToken = &token;
				hadScope = true;
				++braceCount;
			}
			else if (token.value == "}")
			{
				if (braceCount == 0)
				{
					output.addMessage(Output::Level::Error, token.fileName, token.line,
						token.column, false, "encountered } without opening {");
					return false;
				}

				--braceCount;
				if (braceCount == 0 && element == Element::Unknown)
				{
					// End element on last } for elements like functions.
					endElement(stages, tokenRange, i);
					element = Element::Unknown;
					elementStart = true;
					hadScope = false;
				}
			}
			else if (token.value == "]")
			{
				if (squareCount == 0)
				{
					output.addMessage(Output::Level::Error, token.fileName, token.line,
						token.column, false, "encountered ] without opening [");
					return false;
				}

				--squareCount;
				if (inStageDecl && squareCount == 0)
				{
					inStageDecl = false;
					tokenRange.start = i + 1;
				}
			}
			else if (token.value == ";" && parenCount == 0 && braceCount == 0 && squareCount == 0)
			{
				// End element ; outside of (, {, and [ block.
				endElement(stages, tokenRange, i);
				element = Element::Unknown;
				elementStart = true;
				hadScope = false;
			}
			else if (!hadScope && token.value == "uniform")
				element = Element::Uniform;
			else if (!hadScope && token.value == "buffer")
				element = Element::Buffer;
			else if (!hadScope && token.value == "struct")
				element = Element::Struct;
			else if (!hadScope && token.value == "in")
				element = Element::In;
			else if (!hadScope && token.value == "out")
				element = Element::Out;
		}

		lastToken = &token;
	}

	// Make sure this isn't still in an element or within a (, {, or [ block.
	lastToken = &tokens.back();
	if (parenCount > 0)
	{
		output.addMessage(Output::Level::Error, lastToken->fileName, lastToken->line,
			lastToken->column, false,
			"reached end of file without terminating )");
		if (startParenToken)
			output.addMessage(Output::Level::Error, startParenToken->fileName,
				startParenToken->line, startParenToken->column, true,
				"see opening (");
		return false;
	}

	if (braceCount > 0)
	{
		output.addMessage(Output::Level::Error, lastToken->fileName, lastToken->line,
			lastToken->column, false,
			"reached end of file without terminating }");
		if (startBraceToken)
			output.addMessage(Output::Level::Error, startBraceToken->fileName,
				startBraceToken->line, startBraceToken->column, true,
				"see opening {");
		return false;
	}

	if (squareCount > 0)
	{
		output.addMessage(Output::Level::Error, lastToken->fileName, lastToken->line,
			lastToken->column, false,
			"reached end of file without terminating ]");
		if (startSquareToken)
			output.addMessage(Output::Level::Error, startSquareToken->fileName,
				startSquareToken->line, startSquareToken->column, true,
				"see opening [");
		return false;
	}

	if (!elementStart)
	{
		output.addMessage(Output::Level::Error, lastToken->fileName, lastToken->line,
			lastToken->column, false,
			"unexpected end of file");
		return false;
	}

	return true;
}

std::string Parser::createShaderString(std::vector<LineMapping>& lineMappings,
	const Pipeline& pipeline, Stage stage) const
{
	lineMappings.clear();
	std::string shaderString;

	auto stageIndex = static_cast<unsigned int>(stage);

	bool needsPushConstants =
		!m_elements[static_cast<unsigned int>(Element::FreeUniform)][stageIndex].empty();
	if (m_options & RemoveUniformBlocks)
	{
		needsPushConstants |=
			!m_elements[static_cast<unsigned int>(Element::UniformBlock)][stageIndex].empty();
	}

	// Add precision and struct elements first. This ensures that any type declarations are present
	// before generating the push constant.
	for (const TokenRange& tokenRange :
		m_elements[static_cast<unsigned int>(Element::Precision)][stageIndex])
	{
		addElementString(shaderString, lineMappings, tokenRange, pipeline.entryPoints[stageIndex]);
	}

	for (const TokenRange& tokenRange :
		m_elements[static_cast<unsigned int>(Element::Struct)][stageIndex])
	{
		addElementString(shaderString, lineMappings, tokenRange, pipeline.entryPoints[stageIndex]);
	}

	// Add the push constants.
	if (needsPushConstants)
	{
		if (!shaderString.empty() && shaderString.back() != '\n')
			shaderString += '\n';

		// Add two lines at the start.
		shaderString += "layout(push_constant) uniform Uniforms\n{";
		for (unsigned int i = 0; i < 2; ++i)
		{
			lineMappings.emplace_back();
			lineMappings.back().fileName = "<internal>";
			lineMappings.back().line = 0;
		}

		// Add the free uniforms.
		for (const TokenRange& tokenRange :
			m_elements[static_cast<unsigned int>(Element::FreeUniform)][stageIndex])
		{
			addElementString(shaderString, lineMappings, tokenRange,
				pipeline.entryPoints[stageIndex]);
		}

		// Add the uniform blocks if removing them.
		if (m_options & RemoveUniformBlocks)
		{
			for (const TokenRange& tokenRange :
				m_elements[static_cast<unsigned int>(Element::UniformBlock)][stageIndex])
			{
				addElementString(shaderString, lineMappings, tokenRange,
					pipeline.entryPoints[stageIndex]);
			}
		}

		// Add the end. of the block.
		if (!shaderString.empty() && shaderString.back() != '\n')
			shaderString += '\n';

		shaderString += "} uniforms;";
		lineMappings.emplace_back();
		lineMappings.back().fileName = "<internal>";
		lineMappings.back().line = 0;
	}

	// Add the uniform blocks after the push constants if not removed.
	if (!(m_options & RemoveUniformBlocks))
	{
		for (const TokenRange& tokenRange :
			m_elements[static_cast<unsigned int>(Element::UniformBlock)][stageIndex])
		{
			addElementString(shaderString, lineMappings, tokenRange,
				pipeline.entryPoints[stageIndex]);
		}
	}

	// Add everything else.
	for (const TokenRange& tokenRange :
		m_elements[static_cast<unsigned int>(Element::Default)][stageIndex])
	{
		addElementString(shaderString, lineMappings, tokenRange, pipeline.entryPoints[stageIndex]);
	}

	return shaderString;
}

Parser::Element Parser::getElementType(const TokenRange& tokenRange) const
{
	/*
	 * Care about the following:
	 * - Precision declaration
	 * - struct declarations
	 * - free uniforms that use non-opaque types
	 * - uniform blocks
	 */
	bool isUniform = false;
	const auto& tokens = m_tokens.getTokens();
	for (std::size_t i = 0; i < tokenRange.count; ++i)
	{
		const Token& token = tokens[tokenRange.start + i];
		if (token.value == "precision")
			return Element::Precision;
		else if (token.value == "struct")
			return Element::Struct;
		else if (token.value == "uniform")
		{
			// Need to find out the type of uniform first.
			isUniform = true;
		}
		else if (token.value == "{")
		{
			// If we hit a block, it's either a uniform block or something we don't care about.
			if (isUniform)
				return Element::UniformBlock;
			else
				return Element::Default;
		}

		// If a uniform, check to see if it's an opaque type.
		if (isUniform && opaqueTypes.find(token.value) != opaqueTypes.end())
			return Element::Default;
	}

	// If we reach the end, it's either a free uniform declaration of a non-opaque type or something
	// we don't specifically care about.
	if (isUniform)
		return Element::FreeUniform;
	else
		return Element::Default;
}

void Parser::endElement(std::vector<Stage>& stages, TokenRange& tokenRange, std::size_t index)
{
	std::array<bool, stageCount> addStages;
	if (stages.empty())
		addStages.fill(true);
	else
	{
		addStages.fill(false);
		for (Stage stage : stages)
			addStages[static_cast<int>(stage)] = true;
	}

	tokenRange.count = index + 1 - tokenRange.start;
	Element elementType = getElementType(tokenRange);
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (addStages[i])
			m_elements[static_cast<unsigned int>(elementType)][i].push_back(tokenRange);
	}

	stages.clear();
	tokenRange.start = index + 1;
	tokenRange.count = 0;
}

bool Parser::readPipeline(Output& output, const std::vector<Token>& tokens, std::size_t& i)
{
	// Handle all parsing of the pipeline here. This will not be output for any part of the
	// target GLSL.
	Pipeline pipeline;

	if (!skipWhitespace(output, tokens, i))
		return false;

	// Read the name
	if (tokens[i].type != Token::Type::Identifier)
	{
		output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
			tokens[i].column, false, "unexpected token: " + tokens[i].value);
		return false;
	}

	pipeline.token = &tokens[i];
	pipeline.name = tokens[i].value;
	for (const Pipeline& other : m_pipelines)
	{
		if (other.name == pipeline.name)
		{
			output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
				tokens[i].column, false, "pipeline of name " + pipeline.name +
				" already declared");
			output.addMessage(Output::Level::Error, other.token->fileName, other.token->line,
				other.token->column, true,
				"see other declaration of pipeline " + pipeline.name);
			return false;
		}
	}

	if (!skipWhitespace(output, tokens, ++i))
		return false;

	if (tokens[i].value != "{")
	{
		output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
			tokens[i].column, false, "unexpected token: " + tokens[i].value);
		return false;
	}

	++i;
	KeyValueResult keyValueResult;
	do
	{
		const Token* key = nullptr;
		const Token* value = nullptr;
		keyValueResult = readKeyValue(output, key, value, tokens, i);
		if (keyValueResult != KeyValueResult::Success)
			break;

		ParseResult parseResult = readStage(output, pipeline, *key, *value);
		if (parseResult == ParseResult::Error)
			return false;
		else if (parseResult == ParseResult::NotThisType)
		{
			parseResult = readRenderState(output, pipeline, *key, *value);
			if (parseResult == ParseResult::Error)
				return false;
			else if (parseResult == ParseResult::NotThisType)
			{
				output.addMessage(Output::Level::Error, key->fileName, key->line,
					key->column, false, "unknown pipeline stage or render state name: " +
					key->value);
			}
			return false;
		}
	} while (keyValueResult == KeyValueResult::Success);

	if (keyValueResult == KeyValueResult::Error)
		return false;

	m_pipelines.push_back(std::move(pipeline));
	return true;
}

void Parser::addElementString(std::string& str, std::vector<LineMapping>& lineMappings,
	const TokenRange& tokenRange, const std::string& entryPoint) const
{
	if (tokenRange.count == 0)
		return;

	if (removeUniformBlock(str, lineMappings, tokenRange))
		return;

	bool newline = true;
	const auto& tokens = m_tokens.getTokens();

	unsigned int parenCount = 0;
	unsigned int braceCount = 0;
	unsigned int squareCount = 0;

	std::size_t maxValue = std::min(tokenRange.start + tokenRange.count, tokens.size());
	for (std::size_t i = tokenRange.start; i < maxValue; ++i)
	{
		const Token& token = tokens[i];
		if (newline && token.value == "\n")
			continue;

		if (newline)
		{
			if (!str.empty() && str.back() != '\n')
				str += '\n';
			lineMappings.emplace_back();
			lineMappings.back().fileName = token.fileName;
			lineMappings.back().line = token.line;
			newline = false;
		}

		if (token.value == "\n")
			newline = true;
		else if (token.value == "(")
			++parenCount;
		else if (token.value == ")")
			--parenCount;
		else if (token.value == "{")
			++braceCount;
		else if (token.value == "}")
			--braceCount;
		else if (token.value == "[")
			++squareCount;
		else if (token.value == "]")
			--squareCount;

		// Replace entry point name at global scope with "main".
		if (parenCount == 0 && braceCount == 0 && squareCount == 0 && token.value == entryPoint)
			str += "main";
		else
			str += token.value;
	}
}

bool Parser::removeUniformBlock(std::string& str, std::vector<LineMapping>& lineMappings,
	const TokenRange& tokenRange) const
{
	if (!(m_options & RemoveUniformBlocks))
		return false;

	bool newline = true;
	bool processed = false;
	unsigned int braceCount = 0;
	bool isUniform = false;
	const auto& tokens = m_tokens.getTokens();
	std::size_t maxValue = std::min(tokenRange.start + tokenRange.count, tokens.size());
	for (std::size_t i = tokenRange.start; i < maxValue; ++i)
	{
		const Token& token = tokens[i];
		if (processed)
		{
			if (token.value == "{")
				++braceCount;
			if (token.value == "}")
				--braceCount;

			if (braceCount > 0)
			{
				// Keep the contents of the block itself.
				if (newline && token.value == "\n")
					continue;

				if (newline)
				{
					if (!str.empty() && str.back() != '\n')
						str += '\n';
					lineMappings.emplace_back();
					lineMappings.back().fileName = token.fileName;
					lineMappings.back().line = token.line;
					newline = false;
				}

				if (token.value == "\n")
					newline = true;

				str += token.value;
			}
		}
		else
		{
			// Search for the start of a uniform block, skipping those tokens.
			if (token.value == "uniform")
				isUniform = true;
			else if (token.value == "{")
			{
				if (!isUniform)
					return false;

				processed = true;
				++braceCount;
			}
		}
	}

	return processed;
}

} // namespace msl
