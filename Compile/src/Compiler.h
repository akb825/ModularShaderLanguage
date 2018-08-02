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
#include "Parser.h"
#include <memory>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "glslang/Public/ShaderLang.h"

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic pop
#endif

namespace msl
{

using namespace compile;

// Export for tests.
class MSL_COMPILE_EXPORT Compiler
{
public:
	class MSL_COMPILE_EXPORT Stages
	{
	public:
		Stages();
		~Stages();

		std::array<std::unique_ptr<glslang::TShader>, stageCount> shaders;
	};

	class MSL_COMPILE_EXPORT Program
	{
	public:
		Program();
		~Program();

	private:
		friend class Compiler;
		std::unique_ptr<glslang::TProgram> program;
	};

	enum ProcessOptions
	{
		RemapVariables = 0x1,
		DeadCodeElimination = 0x2,
		Optimize = 0x4,
		StripDebug = 0x8
	};

	using SpirV = std::vector<std::uint32_t>;

	static void initialize();
	static void shutdown();

	static const TBuiltInResource& getDefaultResources();

	static bool compile(Stages& stages, Output& output, const std::string& baseFileName,
		const std::string& glsl, const std::vector<Parser::LineMapping>& lineMappings,
		Stage stage, const TBuiltInResource& resources);

	static bool link(Program& program, Output& output, const Parser::Pipeline& pipeline,
		const Stages& stages);

	static SpirV assemble(Output& output, const Program& program, Stage stage,
		const Parser::Pipeline& pipeline);

	static void process(SpirV& spirv, int processOptions);
};

} // namespace msl
