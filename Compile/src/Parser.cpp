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

static bool getStage(Output& output, Parser::Stage& stage, const Token& token)
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

	output.addMessage(Output::Level::Error, token.fileName, token.line,
		token.column, false, "unknown stage type: " + token.value);
	return false;
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
				if (!getStage(output, stage, token))
					return false;

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

	bool endBlock = false;
	++i;
	do
	{
		// Read the contents of the block
		if (!skipWhitespace(output, tokens, i))
			return false;

		if (tokens[i].value == ";")
		{
			// Empty ;
			++i;
			continue;
		}
		else if (tokens[i].value == "}")
		{
			// End of pipeline block
			endBlock = true;
			++i;
			break;
		}
		else
		{
			// stage = entryPoint;
			Stage stage;
			if (!getStage(output, stage, tokens[i]))
				return false;

			if (!skipWhitespace(output, tokens, ++i))
				return false;

			if (tokens[i].value != "=")
			{
				output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
					tokens[i].column, false, "unexpected token: " + tokens[i].value);
				return false;
			}

			if (!skipWhitespace(output, tokens, ++i))
				return false;

			if (tokens[i].type != Token::Type::Identifier)
			{
				output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
					tokens[i].column, false, "unexpected token: " + tokens[i].value);
				return false;
			}
			pipeline.entryPoints[static_cast<int>(stage)] = tokens[i].value;

			if (!skipWhitespace(output, tokens, ++i))
				return false;

			if (tokens[i].value != ";")
			{
				output.addMessage(Output::Level::Error, tokens[i].fileName, tokens[i].line,
					tokens[i].column, false, "unexpected token: " + tokens[i].value);
				return false;
			}

			++i;
		}
	} while (i < tokens.size());

	if (!endBlock)
	{
		const Token& lastToken = tokens.back();
		output.addMessage(Output::Level::Error, lastToken.fileName, lastToken.line,
			lastToken.column, false,
			"unexpected end of file");
		return false;
	}

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
