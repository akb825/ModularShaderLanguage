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

#include <MSL/Frontend/Token.h>
#include <memory>
#include <vector>

/**
 * @file
 * @brief Class for lexigraphical analysis, splitting input into tokens.
 */

namespace msl
{

/**
 * @brief Class for splitting input into tokens.
 */
class Lexer
{
public:
	/**
	 * @brief Splits input into tokens.
	 * @param input The input string.
	 * @param start The start index.
	 * @param length The length of the string.
	 * @return The tokens from the input.
	 */
	static std::vector<Token> tokenize(const std::string& input, std::size_t start = 0,
		std::size_t length = std::string::npos);

	/**
	 * @brief Constructs this with the input string and range.
	 * @param input The input string. This will be stored by pointer, so it must remain alive
	 * as long as this is alive.
	 * @param start The start index.
	 * @param length The length of the string.
	 */
	explicit Lexer(const std::string& input, std::size_t start = 0,
		std::size_t length = std::string::npos);

	/**
	 * @brief Move constructor.
	 * @param other The other object to move to this.
	 */
	Lexer(Lexer&& other);

	/**
	 * @brief Move assignment.
	 * @param other The other object to move to this.
	 * @return A reference to this.
	 */
	Lexer& operator=(Lexer&& other);

	~Lexer();
	Lexer(const Lexer&) = delete;
	Lexer& operator=(const Lexer&) = delete;

	/**
	 * @brief Reads the next token.
	 * @param[out] token The token to populate.
	 * @return True if a token was read, false if the end of the file.
	 */
	bool nextToken(Token& token);

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};

} // namespace msl
