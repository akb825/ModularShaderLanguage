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

#include <MSL/Frontend/Parse/Preprocessor.h>
#include <MSL/Frontend/Parse/DefineMap.h>
#include <MSL/Frontend/Parse/Lexer.h>
#include <MSL/Frontend/FileManager.h>
#include <MSL/Frontend/Output.h>
#include <list>
#include <unordered_set>

namespace msl
{

namespace
{

using FileSet = std::unordered_set<const File*>;
using TokenList = std::list<Token>;

class TokenInserter
{
public:
	TokenInserter(std::vector<Token>& dataStruct)
		: m_vector(&dataStruct)
	{
	}

	TokenInserter(std::list<Token>& dataStruct)
		: m_list(&dataStruct)
	{
	}

	template <typename IterT>
	void insert(const IterT& begin, const IterT& end)
	{
		if (m_vector)
			m_vector->insert(m_vector->end(), begin, end);
		else
			m_list->insert(m_list->end(), begin, end);
	}

private:
	std::vector<Token>* m_vector;
	std::list<Token>* m_list;
};

template <typename IterT>
TokenList::iterator replace(TokenList& tokens, const TokenList::iterator& it,
	const IterT& begin, const IterT& end)
{
	return tokens.insert(tokens.erase(it), begin, end);
}

template <typename IterT>
TokenList::iterator replace(TokenList& tokens,
	const TokenList::iterator& replaceStart, const TokenList::iterator& replaceEnd,
	const IterT& begin, const IterT& end)
{
	return tokens.insert(tokens.erase(replaceStart, replaceEnd), begin, end);
}


bool processTokens(TokenInserter& result, Output& output, FileManager& fileManager,
	FileSet& pragmaOnceFiles, const File& file, DefineMap& defines);

TokenList::iterator escapedNewline(TokenList& tokens, TokenList::iterator it, const File& file)
{
	// Escaped newlines concatinate the previous two tokens. Use the lexer to split it into its own
	// set of tokens.
	const Token& token = *it;

	std::string concat;
	if (it != tokens.begin())
	{
		auto prev = it;
		concat = (--prev)->extractValue(file.contents);
	}
	auto next = it;
	if (++next != tokens.end())
		concat += next->extractValue(file.contents);

	std::vector<Token> concatTokens = Lexer::tokenize(token.file, concat);
	// Fix up the token values.
	for (Token& concatToken : concatTokens)
	{
		concatToken.original = token.original;
		concatToken.line = token.line;
		concatToken.column = token.column;
		concatToken.overrideValue = concatToken.extractValue(concat);
	}

	// Replace the current token and continue with the replaced tokens.
	return replace(tokens, it, concatTokens.begin(), concatTokens.end());
}

TokenList::iterator include(TokenList& tokens, TokenList::iterator it, Output& output,
	const File& file, DefineMap& defines)
{
	(void)tokens;
	(void)it;
	(void)output;
	(void)file;
	(void)defines;
	return it;
}

TokenList::iterator substitute(TokenList& tokens, TokenList::iterator it, Output& output,
	const File& file, DefineMap& defines)
{
	(void)tokens;
	(void)it;
	(void)output;
	(void)file;
	(void)defines;
	return it;
}

bool processTokens(TokenInserter& result, Output& output, FileManager& fileManager,
	FileSet& pragmaOnceFiles, const File& file, DefineMap& defines)
{
	// Convert the tokens to a list for easy insertion and removal.
	// Keep a pointer to the original token so we can chain references for substitutions for error
	// reporting.
	TokenList tokenList;
	for (const Token& token : file.tokens)
	{
		tokenList.push_back(token);
		tokenList.back().original = &token;
	}

	TokenInserter recurseInserter(tokenList);
	bool validForPreproc = true;
	// Loop over the token list to modify it.
	for (auto it = tokenList.begin(); it != tokenList.end();)
	{
		const Token& token = *it;
		switch (token.type)
		{
			// Ignore newlines, but allow new preprocessor directives.
			case Token::Type::Newline:
				validForPreproc = true;
				// fall through to next case.

			// Ignore whitespace and comments.
			case Token::Type::Whitespace:
			case Token::Type::Comment:
				it = tokenList.erase(it);
				break;

			case Token::Type::EscapedNewline:
				it = escapedNewline(tokenList, it, file);
				break;

			case Token::Type::Include:
				it = include(tokenList, it, output, file, defines);
				if (it == tokenList.end())
					return false;

			// Do nothing for literals.
			case Token::Type::IntLiteral:
			case Token::Type::FloatLiteral:
			case Token::Type::DoubleLiteral:
				break;

			// Substitute for everything else.
			default:
				it = substitute(tokenList, it, output, file, defines);
				if (it == tokenList.end())
					return false;
				break;
		}
	}

	result.insert(tokenList.begin(), tokenList.end());
	return true;
}

} // namespace

bool Preprocessor::preprocess(std::vector<Token>& result, Output& output, FileManager& fileManager,
	const File& file, const DefineMap& predefines)
{
	if (fileManager.getFileIndex(file) == FileManager::invalidIndex)
		return false;

	result.clear();
	TokenInserter inserter(result);
	FileSet pragmaOnceFiles;
	DefineMap defines = predefines;
	if (!processTokens(inserter, output, fileManager, pragmaOnceFiles, file, defines))
		return false;

	// Check the final tokens to make sure that they are valid. We didn't do this check before due
	// to allowing \ + newline to concatenate tokens.
	bool valid = true;
	for (const Token& token : result)
	{
		std::shared_ptr<const File> file = fileManager.getFile(token.file);
		if (file)
		{
			if (token.addMessage(output, file->path, file->contents))
				valid = false;
		}
		else
		{
			output.addMessage(Output::Level::Error, "unknown", token.line, token.column, false,
				"Internal error: unknown file.");
			return false;
		}
	}

	return valid;
}

} // namespace msl
