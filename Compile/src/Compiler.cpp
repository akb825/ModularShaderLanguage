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

#include "Compiler.h"
#include <MSL/Compile/Output.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/SPVRemapper.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <cstring>

namespace msl
{

static const EShLanguage stageMap[] =
{
	EShLangVertex,
	EShLangTessControl,
	EShLangTessEvaluation,
	EShLangGeometry,
	EShLangFragment,
	EShLangCompute
};

static_assert(sizeof(stageMap)/sizeof(*stageMap) == Parser::stageCount,
	"Stage map has an incorrect number of elements");

static void addToOutput(Output& output, const std::string& baseFileName,
	const std::vector<Parser::LineMapping>& lineMappings, const std::string& infoStr,
	std::size_t defaultLineNumber = 0)
{
	const char* warningPrefix = "WARNING: ";
	const char* errorPrefix = "ERROR: ";
	const char* internalErrorPrefix = "INTERNAL ERROR: ";
	const char* unimplementedPrefix = "UNIMPLEMENTED: ";
	const char* notePrefix = "NOTE: ";
	const char* unknownPrefix = "UNKNOWN ERROR: ";

	for (std::size_t start = 0; start < infoStr.size();)
	{
		std::size_t end = infoStr.find_first_of('\n', start);
		std::size_t curLen;
		if (end == std::string::npos)
			curLen = std::string::npos;
		else
			curLen = end - start;
		std::string curMessage = infoStr.substr(start, curLen);

		std::string prefix;
		Output::Level level = Output::Level::Error;
		if (boost::algorithm::starts_with(curMessage, warningPrefix))
		{
			curMessage.substr(std::strlen(warningPrefix));
			level = Output::Level::Warning;
		}
		else if (boost::algorithm::starts_with(curMessage, errorPrefix))
		{
			curMessage.substr(std::strlen(errorPrefix));
			level = Output::Level::Error;
		}
		else if (boost::algorithm::starts_with(curMessage, internalErrorPrefix))
		{
			curMessage.substr(std::strlen(internalErrorPrefix));
			level = Output::Level::Error;
			prefix = "internal error: ";
		}
		else if (boost::algorithm::starts_with(curMessage, unimplementedPrefix))
		{
			curMessage.substr(std::strlen(unimplementedPrefix));
			level = Output::Level::Error;
			prefix = "unimplemented: ";
		}
		else if (boost::algorithm::starts_with(curMessage, notePrefix))
		{
			curMessage.substr(std::strlen(notePrefix));
			level = Output::Level::Info;
		}
		else if (boost::algorithm::starts_with(curMessage, unknownPrefix))
		{
			curMessage.substr(std::strlen(unknownPrefix));
			level = Output::Level::Error;
		}

		const char* fileName = nullptr;;
		std::size_t lineNumber = 0;
		std::size_t separator = curMessage.find_first_of(':');
		if (separator != std::string::npos)
		{
			std::size_t endLineNumber = curMessage.find_first_of(':', separator + 1);
			if (endLineNumber != std::string::npos)
			{
				try
				{
					lineNumber = boost::lexical_cast<std::size_t>(curMessage.substr(separator + 1,
						endLineNumber - separator - 1));
					if (lineNumber < lineMappings.size())
					{
						fileName = lineMappings[lineNumber].fileName;
						lineNumber = lineMappings[lineNumber].line;
						curMessage = curMessage.substr(endLineNumber + 2);
					}
				} catch (...)
				{
				}
			}
		}

		if (!fileName)
		{
			fileName = baseFileName.c_str();
			lineNumber = defaultLineNumber;
		}

		output.addMessage(level, fileName, lineNumber, 0, false, prefix + curMessage);

		start = end;
		if (start != std::string::npos)
			++start;
	}
}

static bool addToOutput(Output &output, const spv::SpvBuildLogger& logger,
	const std::string& fileName, std::size_t lineNumber)
{
	std::string messages = logger.getAllMessages();
	if (messages.empty())
		return false;

	const char* tbdFeaturePrefix = "TBD functionality: ";
	const char* missingFeaturePrefix = "Missing functionality: ";
	const char* warningPrefix = "warning: ";
	const char* errorPrefix = "error: ";

	for (std::size_t start = 0; start < messages.size();)
	{
		std::size_t end = messages.find_first_of('\n', start);
		std::size_t curLen;
		if (end == std::string::npos)
			curLen = std::string::npos;
		else
			curLen = end - start;
		std::string curMessage = messages.substr(start, curLen);

		std::string prefix;
		Output::Level level = Output::Level::Error;
		if (boost::algorithm::starts_with(curMessage, tbdFeaturePrefix))
		{
			curMessage.substr(std::strlen(tbdFeaturePrefix));
			level = Output::Level::Error;
			prefix = "tbd feature: ";
		}
		else if (boost::algorithm::starts_with(curMessage, missingFeaturePrefix))
		{
			curMessage.substr(std::strlen(errorPrefix));
			level = Output::Level::Error;
			prefix = "missing feature: ";
		}
		else if (boost::algorithm::starts_with(curMessage, warningPrefix))
		{
			curMessage.substr(std::strlen(warningPrefix));
			level = Output::Level::Warning;
		}
		else if (boost::algorithm::starts_with(curMessage, errorPrefix))
		{
			curMessage.substr(std::strlen(errorPrefix));
			level = Output::Level::Error;
		}

		output.addMessage(level, fileName, lineNumber, 0, false, prefix + curMessage);

		start = end;
		if (start != std::string::npos)
			++start;
	}

	return true;
}

bool Compiler::compile(Stages& stages, Output &output, const std::string& baseFileName,
	const std::string& glsl, const std::vector<Parser::LineMapping>& lineMappings,
	Parser::Stage stage, const TBuiltInResource& resources)
{
	const char* glslStr = glsl.c_str();
	std::unique_ptr<glslang::TShader> shader(
		new glslang::TShader(stageMap[static_cast<unsigned int>(stage)]));
	shader->setStrings(&glslStr, 1);

	bool success = shader->parse(&resources, 450, ECoreProfile, true, false, EShMsgDefault);
	addToOutput(output, baseFileName, lineMappings, shader->getInfoLog());

	if (success)
		stages.shaders[static_cast<unsigned int>(stage)] = std::move(shader);
	return success;
}

bool Compiler::link(glslang::TProgram& program, Output& output, const Parser::Pipeline& pipeline,
	const Stages& stages)
{
	for (unsigned int i = 0; i < Parser::stageCount; ++i)
	{
		if (stages.shaders[i])
			program.addShader(stages.shaders[i].get());
	}

	bool success = program.link(EShMsgDefault);
	std::vector<Parser::LineMapping> dummyLineMappings;
	addToOutput(output, pipeline.token->fileName, dummyLineMappings, program.getInfoLog(),
		pipeline.token->line);
	return success;
}

Compiler::SpirV Compiler::assemble(Output& output, const glslang::TProgram& program,
	Parser::Stage stage, const Parser::Pipeline& pipeline)
{
	glslang::TIntermediate* intermediate = program.getIntermediate(
		stageMap[static_cast<unsigned int>(stage)]);

	if (!intermediate)
		return SpirV();

	SpirV spirv;
	spv::SpvBuildLogger logger;
	glslang::GlslangToSpv(*intermediate, spirv, &logger);
	if (addToOutput(output, logger, pipeline.token->fileName, pipeline.token->line))
		return SpirV();

	return spirv;
}

void Compiler::optimize(SpirV& spirv)
{
	spv::spirvbin_t remapper;
	remapper.remap(spirv, spv::spirvbin_t::DCE_ALL | spv::spirvbin_t::OPT_ALL);
}

} // namespace msl
