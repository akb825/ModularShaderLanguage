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

#include <MSL/Frontend/Config.h>
#include <MSL/Frontend/Export.h>
#include <MSL/Frontend/File.h>

/**
 * @file
 * @brief Utilities for preprocessing files.
 */

namespace msl
{

class DefineMap;
class FileManager;
class Output;

/**
 * @brief Class for running the preprocessor on a file.
 */
class MSL_FRONTEND_EXPORT Preprocessor
{
public:
	/**
	 * @brief Preprocesses a file.
	 *
	 * The preprocessor directives will be run on the tokens of the file, with the substitutions,
	 * inclusions, and ommissions applied. Additionally, comments and whitespace will be removed
	 * from the token stream.
	 *
	 * @remark Certain pragmas, such as warning controls, will remain including the terminating
	 * newline.
	 *
	 * @param result The preprocessed tokens.
	 * @param output Warning or error output.
	 * @param fileManager The file manager for loading includes.
	 * @param file The file to preprocess.
	 * @param predefines Pre-defined preprocessor values.
	 * @return False if an error was encountered.
	 */
	static bool preprocess(std::vector<Token>& result, Output& output, FileManager& fileManager,
		const File& file, const DefineMap& predefines);
};

} // namespace msl
