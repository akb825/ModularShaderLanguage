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
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @file
 * @brief Structure for providing a list of pre-defined values.
 */

namespace msl
{

struct File;
struct Token;

/**
 * @brief Mapping from an identifier name to the replacements.
 *
 * The define may have contain parameters to be called as a function. Unless the parameter is
 * used as a concatentation within the macro body, the parameter is pre-scanned and expanded before
 * being passed into the body. The replacement will be scanned and expanded after the substitution.
 * (ignoring any recursive expansions)
 */
class MSL_FRONTEND_EXPORT DefineMap
{
public:
	/**
	 * @brief Structure for a parameter in a defined macro.
	 */
	struct Parameter
	{
		/**
		 * @brief The name of the parameter.
		 */
		std::string name;

		/**
		 * @brief Whether or not the parameter will be pre-scanned.
		 *
		 * This will be false if the parameter is used as part of a concatenation.
		 */
		bool prescan;
	};

	/**
	 * @brief Structure for the information on a define.
	 */
	struct DefineInfo
	{
		/**
		 * @brief The token for the definition.
		 */
		const Token* token;

		/**
		 * @brief Whether or not the macro is treated as a function.
		 */
		bool function;

		/**
		 * @brief The list of parameters.
		 *
		 * This will always be empty if function is false. If this is empty when function is true,
		 * the macro takes no parameters.
		 */
		std::vector<Parameter> parameters;

		/**
		 * @brief The list of replacement tokens.
		 */
		std::vector<Token> replacement;
	};

	/**
	 * @brief Adds a definition.
	 * @param name The name of the macro to define.
	 * @param replacement The replacement for the macro. This will be tokenized.
	 * @return A pair for the definition info for the macro name and a bool for whether or not the
	 * definition was added. If the bool is false, the definition info will be the previously
	 * defined macro. If the definition info is nullptr, an error occurred.
	 */
	std::pair<const DefineInfo*, bool> define(const std::string& name,
		const std::string& replacement);

	/**
	 * @brief Adds a definition.
	 * @param file The file the token belongs to.
	 * @param token The token that defines the macro. This will be kept with the definition info.
	 * @param replacement The list of tokens to replace the macro with. This must already be
	 * cleaned. (no whitespace or comments and escaped newlines already concatenated)
	 * @return A pair for the definition info for the macro name and a bool for whether or not the
	 * definition was added. If the bool is false, the definition info will be the previously
	 * defined macro. If the definition info is nullptr, an error occurred.
	 */
	std::pair<const DefineInfo*, bool> define(const File& file, const Token& token,
		std::vector<Token> replacement);

	/**
	 * @brief Adds a function-like definition.
	 * @param file The file the token belongs to.
	 * @param token The token that defines the macro. This will be kept with the definition info.
	 * @param parameters The parameters that the macro accepts.
	 * @param replacement The list of tokens to replace the macro with. This must already be
	 * cleaned. (no whitespace or comments and escaped newlines already concatenated)
	 * @return A pair for the definition info for the macro name and a bool for whether or not the
	 * definition was added. If the bool is false, the definition info will be the previously
	 * defined macro. If the definition info is nullptr, an error occurred.
	 */
	std::pair<const DefineInfo*, bool> define(const File& file, const Token& token,
		const std::vector<std::string>& parameters, std::vector<Token> replacement);

	/**
	 * @brief Removes a definition.
	 * @param name The name of the definition to remove.
	 * @return True if the definition was removed.
	 */
	inline bool undef(const std::string& name);

	/**
	 * @brief Finds a definition.
	 * @param name The name of the definition to find.
	 * @return The information for the definition, or nullptr if not found.
	 */
	inline const DefineInfo* find(const std::string& name) const;

private:
	std::unordered_map<std::string, DefineInfo> m_defines;
};

inline bool DefineMap::undef(const std::string& name)
{
	return m_defines.erase(name) > 0;
}

inline const DefineMap::DefineInfo* DefineMap::find(const std::string& name) const
{
	auto foundIter = m_defines.find(name);
	if (foundIter == m_defines.end())
		return nullptr;

	return &foundIter->second;
}

} // namespace msl
