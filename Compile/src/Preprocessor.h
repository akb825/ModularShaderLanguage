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
#include "TokenList.h"
#include <istream>
#include <string>
#include <utility>
#include <vector>

namespace msl
{

class Output;

class Preprocessor
{
public:
	void addIncludePath(std::string path);
	void addDefine(std::string name, std::string value);
	bool preprocess(TokenList& tokenList, Output& output, const std::string& fileName) const;
	bool preprocess(TokenList& tokenList, Output& output, std::istream& stream,
		const std::string& fileName) const;

private:
	std::vector<std::string> m_includePaths;
	std::vector<std::pair<std::string, std::string>> m_defines;
};

} // namespace msl
