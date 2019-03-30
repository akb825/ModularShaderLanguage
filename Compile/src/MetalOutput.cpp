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
	std::uint32_t version, bool ios, const std::string& fileName, std::size_t line,
	std::size_t column)
{
	spirv_cross::CompilerMSL::Options options;
	options.platform = ios ? spirv_cross::CompilerMSL::Options::iOS :
		spirv_cross::CompilerMSL::Options::macOS;
	options.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(version/10,
		version%10);
	spirv_cross::CompilerMSL compiler(spirv);
	compiler.set_msl_options(options);
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
