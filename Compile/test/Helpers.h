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

#include "TokenList.h"
#include <boost/filesystem.hpp>
#include <fstream>

extern boost::filesystem::path exeDir;

inline std::string readFile(const boost::filesystem::path& fileName)
{
	std::ifstream stream(fileName.string());
	return std::string(std::istreambuf_iterator<char>(stream.rdbuf()),
		std::istreambuf_iterator<char>());
}

inline std::string tokensToString(const msl::TokenList& tokens)
{
	std::string output;
	for (const msl::Token& token : tokens.getTokens())
		output += token.value;
	return output;
}

inline std::string pathStr(const boost::filesystem::path& path)
{
#if MSL_WINDOWS
	boost::filesystem::path preferredPath = path;
	preferredPath.make_preferred();
	std::string str = preferredPath.string();
	if (preferredPath.is_absolute() && str.size() > 2 && str[2] == '/')
		str[2] = '\\';
	return str;
#else
	return path.string();
#endif
}
