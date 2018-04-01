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
#include <MSL/Compile/Export.h>
#include <MSL/Compile/Types.h>
#include "TokenList.h"
#include <array>
#include <float.h>

namespace msl
{

using namespace compile;

class Output;

// Export for tests.
class MSL_COMPILE_EXPORT Parser
{
public:
	enum Options
	{
		RemoveUniformBlocks = 0x1
	};

	struct Pipeline
	{
		const Token* token = nullptr;
		std::string name;
		std::array<Token, stageCount> entryPoints;
		RenderState renderState;
	};

	struct Sampler
	{
		const Token* token;
		std::string name;
		SamplerState state;
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

	const std::vector<Sampler>& getSamplers() const
	{
		return m_samplers;
	}

	bool parse(Output& output, int options = 0);
	std::string createShaderString(std::vector<LineMapping>& lineMappings, Output& output,
		const Pipeline& pipeline, Stage stage, bool ignoreEntryPoint = false) const;

private:
	enum class Element
	{
		Precision,
		Struct,
		FreeUniform,
		UniformBlock,
		Default
	};

	enum class EntryPointState
	{
		NotFound,
		Replaced,
		MultipleFound
	};

	static const unsigned int elementCount = static_cast<unsigned int>(Element::Default) + 1;

	struct TokenRange
	{
		std::size_t start;
		std::size_t count;
	};

	Element getElementType(const TokenRange& tokenRange) const;
	void endElement(std::vector<Stage>& stages, TokenRange& tokenRange, std::size_t index);
	void endMetaElement(TokenRange& tokenRange, std::size_t index);
	bool readPipeline(Output& output, const std::vector<Token>& tokens, std::size_t& i);
	bool readSampler(Output& output, const std::vector<Token>& tokens, std::size_t& i);
	EntryPointState addElementString(std::string& str, std::vector<LineMapping>& lineMappings,
		const TokenRange& tokenRange, const Token* entryPoint = nullptr) const;
	bool removeUniformBlock(std::string& str, std::vector<LineMapping>& lineMappings,
		const TokenRange& tokenRange) const;

	TokenList m_tokens;

	int m_options;
	std::array<std::array<std::vector<TokenRange>, stageCount>, elementCount> m_elements;
	std::vector<Pipeline> m_pipelines;
	std::vector<Sampler> m_samplers;
};

} // namespace msl
