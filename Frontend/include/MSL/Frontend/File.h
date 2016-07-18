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
#include <MSL/Frontend/Parse/Token.h>
#include <string>
#include <vector>

/**
 * @file
 * @brief Structure with information on a file.
 */

namespace msl
{

/**
* @brief Structure defining the information within a file.
*/
struct File
{
	/**
	 * @brief The full path of the file.
	 *
	 * This is the file name combined with the include path. This isn't necessarily an absoulte
	 * path.
	 */
	std::string path;

	/**
	 * @brief The contents of the file.
	 */
	std::string contents;

	/**
	 * @brief The tokens from the file contents.
	 */
	std::vector<Token> tokens;
};

} // namespace msl
