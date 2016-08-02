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

namespace msl
{

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
	for (auto& elements : m_elements)
		elements.clear();
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
	for (const TokenRange& tokenRange : m_elements[stageIndex])
		addElementString(shaderString, lineMappings, tokenRange, pipeline.entryPoints[stageIndex]);

	return shaderString;
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
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (addStages[i])
			m_elements[i].push_back(tokenRange);
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

	// Check to see if this is a uniform block.
	bool isUniform = false;
	unsigned int parenCount = 0;
	unsigned int braceCount = 0;
	unsigned int squareCount = 0;
	const auto& tokens = m_tokens.getTokens();
	std::size_t maxValue = std::min(tokenRange.start + tokenRange.count, tokens.size());
	for (std::size_t i = tokenRange.start; i < maxValue; ++i)
	{
		const Token& token = tokens[i];
		if (token.value == "uniform")
			isUniform = true;
		else if (token.value == "{")
		{
			if (!isUniform)
				return false;

			++braceCount;
			break;
		}
	}

	if (braceCount == 0)
		return false;

	braceCount = 0;
	bool start = true;
	bool newline = true;
	bool startLayout = false;
	bool startLayoutOpenParen = false;
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

		// Make uniform blocks use push_constant blocks. Uniform blocks are required are required
		// for GLSL Vulkan, but push_constants are equivalent to individual uniforms.
		if (start)
		{
			str += "layout(push_constant) ";
			start = false;
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

		if (startLayout && !newline)
		{
			if (parenCount == 0)
			{
				if (startLayoutOpenParen)
					startLayout = false;
			}
			else
				startLayoutOpenParen = true;
			continue;
		}
		else if (parenCount == 0 && braceCount == 0 && squareCount == 0 && token.value == "layout")
		{
			startLayout = true;
			continue;
		}
		str += token.value;
	}
	return true;
}

} // namespace msl
