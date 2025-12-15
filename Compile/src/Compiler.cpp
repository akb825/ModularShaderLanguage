/*
 * Copyright 2016-2025 Aaron Barany
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
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <atomic>
#include <cstring>

#include <spirv-tools/optimizer.hpp>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#elif MSL_MSC
#pragma warning(push)
#pragma warning(disable: 4018)
#endif

#include "SPIRV/GlslangToSpv.h"
#include "glslang/Public/ResourceLimits.h"

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic pop
#elif MSL_MSC
#pragma warning(pop)
#endif

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

static_assert(sizeof(stageMap)/sizeof(*stageMap) == stageCount,
	"Stage map has an incorrect number of elements");

static EShMessages glslMessages =
	static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

static std::atomic<unsigned int> initCounter;

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

	const char* ignoreMessages[] =
	{
		"No code generated.",
		"most version-specific features are present, but some are missing.",
		"stage:",
		"all default precisions are highp; use precision statements to quiet warning, e.g.:",
		"\"precision mediump int; precision highp float;\""
	};

	for (std::size_t start = 0; start < infoStr.size();)
	{
		std::size_t end = infoStr.find_first_of('\n', start);
		std::size_t curLen;
		if (end == std::string::npos)
			curLen = std::string::npos;
		else
			curLen = end - start;
		std::string curMessage = infoStr.substr(start, curLen);
		boost::algorithm::trim(curMessage);

		bool ignored = false;
		for (const char* suffix : ignoreMessages)
		{
			if (boost::algorithm::ends_with(curMessage, suffix))
			{
				ignored = true;
				break;
			}
		}

		if (curMessage.empty() || ignored)
		{
			start = end;
			if (start != std::string::npos)
				++start;
			continue;
		}

		std::string prefix;
		Output::Level level = Output::Level::Info;
		if (boost::algorithm::starts_with(curMessage, warningPrefix))
		{
			curMessage = curMessage.substr(std::strlen(warningPrefix));
			level = Output::Level::Warning;
		}
		else if (boost::algorithm::starts_with(curMessage, errorPrefix))
		{
			curMessage = curMessage.substr(std::strlen(errorPrefix));
			level = Output::Level::Error;
		}
		else if (boost::algorithm::starts_with(curMessage, internalErrorPrefix))
		{
			curMessage = curMessage.substr(std::strlen(internalErrorPrefix));
			level = Output::Level::Error;
			prefix = "internal error: ";
		}
		else if (boost::algorithm::starts_with(curMessage, unimplementedPrefix))
		{
			curMessage = curMessage.substr(std::strlen(unimplementedPrefix));
			level = Output::Level::Error;
			prefix = "unimplemented: ";
		}
		else if (boost::algorithm::starts_with(curMessage, notePrefix))
		{
			curMessage = curMessage.substr(std::strlen(notePrefix));
			level = Output::Level::Info;
		}
		else if (boost::algorithm::starts_with(curMessage, unknownPrefix))
		{
			curMessage = curMessage.substr(std::strlen(unknownPrefix));
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
						endLineNumber - separator - 1)) - 1;
					if (lineNumber < lineMappings.size())
					{
						fileName = lineMappings[lineNumber].fileName;
						lineNumber = lineMappings[lineNumber].line;
						curMessage = curMessage.substr(endLineNumber + 2);
					}
				}
				catch (...)
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
	const std::string& fileName, std::size_t line, std::size_t column)
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
		boost::algorithm::trim(curMessage);

		std::string prefix;
		Output::Level level = Output::Level::Error;
		if (boost::algorithm::starts_with(curMessage, tbdFeaturePrefix))
		{
			curMessage = curMessage.substr(std::strlen(tbdFeaturePrefix));
			level = Output::Level::Error;
			prefix = "tbd feature: ";
		}
		else if (boost::algorithm::starts_with(curMessage, missingFeaturePrefix))
		{
			curMessage = curMessage.substr(std::strlen(errorPrefix));
			level = Output::Level::Error;
			prefix = "missing feature: ";
		}
		else if (boost::algorithm::starts_with(curMessage, warningPrefix))
		{
			curMessage = curMessage.substr(std::strlen(warningPrefix));
			level = Output::Level::Warning;
		}
		else if (boost::algorithm::starts_with(curMessage, errorPrefix))
		{
			curMessage = curMessage.substr(std::strlen(errorPrefix));
			level = Output::Level::Error;
		}

		output.addMessage(level, fileName, line, column, false, prefix + curMessage);

		start = end;
		if (start != std::string::npos)
			++start;
	}

	return true;
}

Compiler::Stages::Stages()
{
}

Compiler::Stages::~Stages()
{
}

Compiler::Program::Program()
	: program(new glslang::TProgram)
{
}

Compiler::Program::~Program()
{
}

void Compiler::initialize()
{
	if (initCounter++ == 0)
		glslang::InitializeProcess();
}

void Compiler::shutdown()
{
	if (--initCounter == 0)
		glslang::FinalizeProcess();
}

const TBuiltInResource& Compiler::getDefaultResources()
{
	return *GetDefaultResources();;
}

bool Compiler::compile(Stages& stages, Output &output, const std::string& baseFileName,
	const std::string& glsl, const std::vector<Parser::LineMapping>& lineMappings,
	Stage stage, const TBuiltInResource& resources, std::uint32_t spirvVersion)
{
	const char* glslStr = glsl.c_str();
	std::unique_ptr<glslang::TShader> shader(
		new glslang::TShader(stageMap[static_cast<unsigned int>(stage)]));
	shader->setEnvTarget(glslang::EShTargetSpv,
		static_cast<glslang::EShTargetLanguageVersion>(spirvVersion));
	shader->setStrings(&glslStr, 1);
	shader->setAutoMapBindings(true);
	shader->setAutoMapLocations(true);

	bool success = shader->parse(&resources, 450, ECoreProfile, true, false, glslMessages);
	addToOutput(output, baseFileName, lineMappings, shader->getInfoLog());

	if (success)
		stages.shaders[static_cast<unsigned int>(stage)] = std::move(shader);
	return success;
}

bool Compiler::link(Program& program, Output& output, const Parser::Pipeline& pipeline,
	const Stages& stages)
{
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (stages.shaders[i])
			program.program->addShader(stages.shaders[i].get());
	}

	bool success = program.program->link(glslMessages);
	std::vector<Parser::LineMapping> dummyLineMappings;
	addToOutput(output, pipeline.token->fileName, dummyLineMappings, program.program->getInfoLog(),
		pipeline.token->line);
	return success;
}

Compiler::SpirV Compiler::assemble(Output& output, const Program& program,
	Stage stage, const Parser::Pipeline& pipeline)
{
	glslang::TIntermediate* intermediate = program.program->getIntermediate(
		stageMap[static_cast<unsigned int>(stage)]);

	if (!intermediate)
		return SpirV();

	SpirV spirv;
	spv::SpvBuildLogger logger;
	glslang::SpvOptions options;
	options.generateDebugInfo = true;
	glslang::GlslangToSpv(*intermediate, spirv, &logger);
	if (addToOutput(output, logger, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column))
	{
		return SpirV();
	}

	return spirv;
}

void Compiler::process(SpirV& spirv, int processOptions)
{
	if (processOptions == 0)
		return;

	// NOTE: We have some known invalid code, such as missing bindings. Therefore we need to skip
	// validation.
	spv_optimizer_options options = spvOptimizerOptionsCreate();
	spvOptimizerOptionsSetRunValidator(options, false);
	spvtools::Optimizer optimizer(SPV_ENV_VULKAN_1_0);
	if (processOptions & RemapVariables)
		optimizer.RegisterPass(spvtools::CreateCanonicalizeIdsPass());
	if (processOptions & DeadCodeElimination)
	{
		optimizer.RegisterPass(spvtools::CreateEliminateDeadFunctionsPass());
		optimizer.RegisterPass(spvtools::CreateEliminateDeadConstantPass());
	}
	if (processOptions & StripDebug)
		optimizer.RegisterPass(spvtools::CreateStripDebugInfoPass());
	if (processOptions & Optimize)
		optimizer.RegisterPerformancePasses();
	SpirV optimizedSpirV;
	if (optimizer.Run(spirv.data(), spirv.size(), &optimizedSpirV, options))
		spirv = std::move(optimizedSpirV);
	spvOptimizerOptionsDestroy(options);
}

} // namespace msl
