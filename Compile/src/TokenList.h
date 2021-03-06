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
#include "Token.h"
#include <memory>
#include <set>
#include <vector>

namespace msl
{

class TokenList
{
public:
	const std::vector<Token>& getTokens() const
	{
		return m_tokens;
	}

private:
	friend class Preprocessor;

	const char* stringPtr(std::string str)
	{
		return m_strings.emplace(std::move(str)).first->c_str();
	}

	std::vector<Token> m_tokens;
	std::set<std::string> m_strings;
};

} // namespace msl
