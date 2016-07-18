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

#include <MSL/Frontend/Parse/DefineMap.h>
#include <MSL/Frontend/Parse/Lexer.h>
#include <MSL/Frontend/File.h>
#include <algorithm>

namespace msl
{

std::pair<const DefineMap::DefineInfo*, bool> DefineMap::define(const std::string& name,
	const std::string& replacement)
{
	auto foundIter = m_defines.find(name);
	if (foundIter != m_defines.end())
		return std::make_pair(&foundIter->second, false);

	DefineInfo defineInfo;
	defineInfo.token = nullptr;
	defineInfo.function = false;
	defineInfo.replacement = Lexer::tokenize(0, replacement);
	return std::make_pair(&m_defines.emplace(name, std::move(defineInfo)).first->second, true);
}

std::pair<const DefineMap::DefineInfo*, bool> DefineMap::define(const File& file, const Token& token,
	std::vector<Token> replacement)
{
	std::string name = token.extractValue(file.contents);
	auto foundIter = m_defines.find(name);
	if (foundIter != m_defines.end())
		return std::make_pair(&foundIter->second, false);

	for (const Token& token : replacement)
	{
		Token::Category category = Token::getCategory(token.type);
		if (category == Token::Category::Whitespace || category == Token::Category::Comment)
			return std::make_pair(nullptr, false);
	}

	DefineInfo defineInfo;
	defineInfo.token = &token;
	defineInfo.function = false;
	defineInfo.replacement = std::move(replacement);
	return std::make_pair(&m_defines.emplace(name, std::move(defineInfo)).first->second, true);
}

std::pair<const DefineMap::DefineInfo*, bool> DefineMap::define(const File& file, const Token& token,
	const std::vector<std::string>& parameters, std::vector<Token> replacement)
{
	std::string name = token.extractValue(file.contents);
	auto foundIter = m_defines.find(name);
	if (foundIter != m_defines.end())
		return std::make_pair(&foundIter->second, false);

	for (const Token& token : replacement)
	{
		Token::Category category = Token::getCategory(token.type);
		if (category == Token::Category::Whitespace || category == Token::Category::Comment)
			return std::make_pair(nullptr, false);
	}

	DefineInfo defineInfo;
	defineInfo.token = &token;
	defineInfo.function = false;
	defineInfo.parameters.resize(parameters.size());
	for (std::size_t i = 0; i < parameters.size(); ++i)
	{
		defineInfo.parameters.emplace_back();
		defineInfo.parameters.back().name = parameters[i];
		defineInfo.parameters.back().prescan = true;
	}
	defineInfo.replacement = std::move(replacement);

	// Check if any parameters are concatenated to disable prescan.
	for (auto it = defineInfo.replacement.begin(); it != defineInfo.replacement.end(); ++it)
	{
		bool isConcat = false;

		if (it != defineInfo.replacement.begin())
		{
			auto prev = it;
			if ((--prev)->type == Token::Type::PreprocConcat)
				isConcat = true;
		}
		auto next = it;
		if (++next != defineInfo.replacement.end() && next->type == Token::Type::PreprocConcat)
			isConcat = true;

		if (isConcat)
		{
			std::string value = token.extractValue(file.contents);
			auto foundIter = std::find(parameters.begin(), parameters.end(), value);
			if (foundIter != parameters.end())
				defineInfo.parameters[foundIter - parameters.begin()].prescan = false;
		}
	}

	return std::make_pair(&m_defines.emplace(name, std::move(defineInfo)).first->second, true);
}

} // namespace msl
