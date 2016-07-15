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
#include <string>

/**
 * @file
 * @brief Utility functions for manipulating paths.
 */

namespace msl
{

/**
 * @brief Collection of utility functions for manipulating paths.
 */
class MSL_FRONTEND_EXPORT Path
{
public:
#if MSL_WINDOWS
	static const char pathSeparator = '\\';
	static const char otherPathSeparator = '/';
#else
	/**
	 * @brief Constant for the primary path separator.
	 */
	static const char pathSeparator = '/';

	/**
	 * @brief Constant for the secondary path separator, or 0 if no secondary separator.
	 */
	static const char otherPathSeparator = 0;
#endif

	/**
	 * @brief Returns whether or not the path is absolute.
	 * @param path The path the check.
	 * @return True if the path is absolute.
	 */
	static bool isAbsolute(const std::string& path);

	/**
	 * @brief Combines two paths.
	 * @param path1 The first path.
	 * @param path2 The second path.
	 * @return The combined path.
	 */
	static std::string combine(const std::string& path1, const std::string& path2);

	/**
	 * @brief Gets the directory portion of a path.
	 * @param path The path to get the directory from.
	 * @return The directory or empty string if no directory portion.
	 */
	static std::string getDirectory(const std::string& path);

	/**
	 * @brief Gets the file potion of a path.
	 * @param path The path to get the file from.
	 * @return The file or empty string if no file portion.
	 */
	static std::string getFile(const std::string& path);

	/**
	 * @brief Normalizes a path, making the separators consistent, removing extra separators, and
	 * removing extra . and .. elements.
	 * @param path The path to normalize.
	 * @return The normalized path.
	 */
	static std::string normalize(const std::string& path);
};

} // namespace msl
