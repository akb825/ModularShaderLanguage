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

#include <MSL/Frontend/Path.h>

namespace msl
{

bool Path::isAbsolute(const std::string& path)
{
	if (path.empty())
		return false;

	if (path[0] == pathSeparator || path[0] == otherPathSeparator)
		return true;

#if MSL_WINDOWS
	if (path.size() > 1 && path[1] == ':')
		return true;
#endif

	return false;
}

std::string Path::combine(const std::string& path1, const std::string& path2)
{
	if (path2.empty())
		return path1;
	else if (path1.empty())
		return path2;

	if (path1.back() == pathSeparator || path2.back() == otherPathSeparator)
		return path1 + path2;
	else
	{
		std::string combinedPath = path1;
		combinedPath += pathSeparator;
		combinedPath += path2;
		return combinedPath;\
	}
}

std::string Path::getDirectory(const std::string& path)
{
	const char separators[] = {pathSeparator, otherPathSeparator, 0};
	std::size_t index = path.find_last_of(separators);
	if (index == std::string::npos)
		return std::string();

	return path.substr(0, index);
}

std::string Path::getFile(const std::string& path)
{
	const char separators[] = {pathSeparator, otherPathSeparator, 0};
	std::size_t index = path.find_last_of(separators);
	if (index == std::string::npos)
		return path;
	else if (index == path.size() - 1)
		return std::string();

	return path.substr(index + 1);
}

} // namespace msl
