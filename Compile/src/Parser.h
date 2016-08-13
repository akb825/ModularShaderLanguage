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

#pragma once

#include <MSL/Config.h>
#include "TokenList.h"
#include <array>

namespace msl
{

class Output;

class Parser
{
public:
	enum Options
	{
		RemoveUniformBlocks = 0x1
	};

	enum class Stage
	{
		Vertex,
		TessellationControl,
		TessellationEvaluation,
		Geometry,
		Fragment,
		Compute
	};

	static const unsigned int stageCount = static_cast<unsigned int>(Stage::Compute) + 1;

	struct Pipeline
	{
		const Token* token;
		std::string name;
		std::array<std::string, stageCount> entryPoints;
	};

	struct LineMapping
	{
		const char* fileName;
		std::size_t line;
	};

	Parser()
		: m_options(0)
	{
	}

	TokenList& getTokens()
	{
		return m_tokens;
	}

	const std::vector<Pipeline>& getPipelines() const
	{
		return m_pipelines;
	}

	bool parse(Output& output, int options = 0);
	std::string createShaderString(std::vector<LineMapping>& lineMappings, const Pipeline& pipeline,
		Stage stage) const;

private:
	struct TokenRange
	{
		std::size_t start;
		std::size_t count;
	};

	void endElement(std::vector<Stage>& stages, TokenRange& tokenRange, std::size_t index);
	bool readPipeline(Output& output, const std::vector<Token>& tokens, std::size_t& i);
	void addElementString(std::string& str, std::vector<LineMapping>& lineMappings,
		const TokenRange& tokenRange, const std::string& entryPoint) const;
	bool removeUniformBlock(std::string& str, std::vector<LineMapping>& lineMappings,
		const TokenRange& tokenRange) const;

	TokenList m_tokens;

	int m_options;
	std::array<std::vector<TokenRange>, stageCount> m_elements;
	std::vector<Pipeline> m_pipelines;
};

} // namespace msl
