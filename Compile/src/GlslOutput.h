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
#include <MSL/Compile/TargetGlsl.h>
#include "Compiler.h"

namespace msl
{

class GlslOutput
{
public:
	struct Options
	{
		unsigned int version = 0;
		bool es = false;
		bool remapDepthRange = false;
		bool vulkanSemantics = false;
		TargetGlsl::Precision defaultFloatPrecision = TargetGlsl::Precision::None;
		TargetGlsl::Precision defaultIntPrecision = TargetGlsl::Precision::None;
		std::vector<std::string> headerLines;
		std::vector<std::string> requiredExtensions;
	};

	static std::string disassemble(Output& output, const Compiler::SpirV& spirv,
		const Options& options, const std::string& fileName, std::size_t line, std::size_t column);
};

} // namespace msl
