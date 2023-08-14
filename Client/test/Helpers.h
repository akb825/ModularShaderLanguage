/*
 * Copyright 2016-2023 Aaron Barany
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

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#if MSL_CLANG
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
#endif

#include <boost/filesystem/path.hpp>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic pop
#endif

extern boost::filesystem::path exeDir;

inline std::string pathStr(const boost::filesystem::path& path)
{
#if MSL_WINDOWS
	std::string str = path.string();
	if (path.is_absolute() && str.size() > 2 && str[2] == '/')
		str[2] = '\\';
	return str;
#else
	return path.string();
#endif
}
