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
#include <string>

namespace msl
{

struct Token
{
	enum class Type
	{
		Unknown,
		Whitespace,
		Identifier,
		Symbol,
		IntLiteral,
		FloatLiteral,
		BoolLiteral
	};

	Token()
		: type(Type::Unknown)
		, fileName(nullptr)
		, line(0)
		, column(0)
	{
	}

	Token(Type type_, std::string value_, const char* fileName_, std::size_t line_,
		std::size_t column_)
		: type(type_)
		, value(std::move(value_))
		, fileName(fileName_)
		, line(line_)
		, column(column_)
	{
	}

	Type type;
	std::string value;
	const char* fileName;
	std::size_t line;
	std::size_t column;
};

} // namespace msl
