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

#include "MetalOutput.h"
#include "spirv_msl.hpp"
#include <MSL/Compile/Output.h>

namespace msl
{

std::string MetalOutput::disassemble(Output& output, const Compiler::SpirV& spirv,
	const Parser::Pipeline& pipeline, const Options& options)
{
	spirv_cross::MSLConfiguration compilerOptions;
	compilerOptions.flip_vert_y = options.flipVertexY;
	compilerOptions.flip_frag_y = options.flipFragmentY;

	spirv_cross::CompilerGLSL::Options glslOptions;
	glslOptions.vertex.fixup_clipspace = options.remapDepthRange;

	spirv_cross::CompilerMSL compiler(spirv);
	compiler.set_options(glslOptions);
	try
	{
		return compiler.compile(compilerOptions);
	}
	catch (const spirv_cross::CompilerError& e)
	{
		output.addMessage(Output::Level::Error, pipeline.token->fileName, pipeline.token->line,
			pipeline.token->column, false, e.what());
		return std::string();
	}
}

} // namespace msl
