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

#include <MSL/Frontend/Parse/Token.h>
#include <ostream>
#include <vector>

namespace msl
{

inline std::ostream& operator<<(std::ostream& stream, const msl::Token& token)
{
	return stream << "{type: " << (int)token.type << ", start: " << token.start <<
		", length: " << token.length << ", line: " << token.line <<
		", column: " << token.column << "}";
}

inline void addToken(std::vector<Token>& tokens, const std::string& input, Token::Type type,
	const std::string& str, std::size_t file = 0)
{
	if (tokens.empty())
		tokens.emplace_back(type, file, 0, str.size(), 0, 0);
	else
	{
		const Token& lastToken = tokens.back();
		std::size_t line = lastToken.line;
		std::size_t column = lastToken.column;
		for (std::size_t i = 0; i < lastToken.length; ++i)
		{
			if (input[i + lastToken.start] == '\n')
			{
				++line;
				column = 0;
			}
			else
				++column;
		}

		tokens.emplace_back(type, file, lastToken.start + lastToken.length, str.size(), line,
			column);
	}
}

} // namespace msl
