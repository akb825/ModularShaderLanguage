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

#include "Parser.h"
#include "glslang/Public/ShaderLang.h"
#include <memory>

namespace msl
{

class Compiler
{
public:
	struct Stages
	{
		std::array<std::unique_ptr<glslang::TShader>, Parser::stageCount> shaders;
	};

	enum ProcessOptions
	{
		RemapVariables,
		Optimize,
		StripDebug
	};

	using SpirV = std::vector<std::uint32_t>;

	static void initialize();
	static void shutdown();

	static bool compile(Stages& stages, Output& output, const std::string& baseFileName,
		const std::string& glsl, const std::vector<Parser::LineMapping>& lineMappings,
		Parser::Stage stage, const std::string& entryPoint, const TBuiltInResource& resources);

	static bool link(glslang::TProgram& program, Output& output, const Parser::Pipeline& pipeline,
		const Stages& stages);

	static SpirV assemble(Output& output, const glslang::TProgram& program, Parser::Stage stage,
		const Parser::Pipeline& pipeline);

	static void process(SpirV& spirv, int processOptions);
};

} // namespace msl
