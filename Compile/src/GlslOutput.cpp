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

#include "GlslOutput.h"
#include "spirv_glsl.hpp"
#include <MSL/Compile/Output.h>

namespace msl
{

std::string GlslOutput::disassemble(Output& output, const Compiler::SpirV& spirv,
	const Options& options, const std::string& fileName, std::size_t line, std::size_t column)
{
	spirv_cross::CompilerGLSL::Options compilerOptions;
	compilerOptions.version = options.version;
	compilerOptions.es = options.es;
	compilerOptions.vertex.fixup_clipspace = options.remapDepthRange;
	compilerOptions.vulkan_semantics = options.vulkanSemantics;
	compilerOptions.fragment.default_float_precision =
		static_cast<spirv_cross::CompilerGLSL::Options::Precision>(options.defaultFloatPrecision);
	compilerOptions.fragment.default_int_precision =
		static_cast<spirv_cross::CompilerGLSL::Options::Precision>(options.defaultIntPrecision);

	spirv_cross::CompilerGLSL compiler(spirv);
	compiler.set_options(compilerOptions);
	for (const std::string& header : options.headerLines)
		compiler.add_header_line(header);
	for (const std::string& extension : options.requiredExtensions)
		compiler.require_extension(extension);

	try
	{
		return compiler.compile();
	}
	catch (const spirv_cross::CompilerError& e)
	{
		output.addMessage(Output::Level::Error, fileName, line, column, false, e.what());
		return std::string();
	}
}

} // namespace msl
