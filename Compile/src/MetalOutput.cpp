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

#include "MetalOutput.h"
#include <MSL/Compile/Output.h>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "spirv_msl.hpp"

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic pop
#endif

namespace msl
{

std::string MetalOutput::disassemble(Output& output, const Compiler::SpirV& spirv, Stage stage,
	std::uint32_t version, bool ios, bool outputToBuffer, bool hasPushConstant,
	std::uint32_t bufferCount, std::uint32_t textureCount, const std::string& fileName,
	std::size_t line, std::size_t column)
{
	spirv_cross::CompilerMSL::Options options;
	options.platform = ios ? spirv_cross::CompilerMSL::Options::iOS :
		spirv_cross::CompilerMSL::Options::macOS;
	options.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(version/100,
		version%100);
	options.capture_output_to_buffer = outputToBuffer;

	spirv_cross::CompilerMSL compiler(spirv);
	compiler.set_msl_options(options);

	spv::ExecutionModel executionModel = spv::ExecutionModelMax;
	switch (stage)
	{
		case Stage::Vertex:
			executionModel = spv::ExecutionModelVertex;
			break;
		case Stage::TessellationControl:
			executionModel = spv::ExecutionModelTessellationControl;
			break;
		case Stage::TessellationEvaluation:
			executionModel = spv::ExecutionModelTessellationEvaluation;
			break;
		case Stage::Geometry:
			executionModel = spv::ExecutionModelGeometry;
			break;
		case Stage::Fragment:
			executionModel = spv::ExecutionModelFragment;
			break;
		case Stage::Compute:
			executionModel = spv::ExecutionModelGLCompute;
			break;
	}

	for (std::uint32_t i = hasPushConstant ? 1 : 0; i < bufferCount; ++i)
	{
		spirv_cross::MSLResourceBinding binding;
		binding.stage = executionModel;
		binding.desc_set = 0;
		binding.binding = i;
		binding.msl_buffer = i;
		compiler.add_msl_resource_binding(binding);
	}

	for (std::uint32_t i = 0; i < textureCount; ++i)
	{
		spirv_cross::MSLResourceBinding binding;
		binding.stage = executionModel;
		binding.desc_set = 1;
		binding.binding = i;
		binding.msl_texture = i;
		binding.msl_sampler = i;
		compiler.add_msl_resource_binding(binding);
	}

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
