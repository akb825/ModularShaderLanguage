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

#include "Preprocessor.h"
#include <MSL/Compile/Output.h>

#if MSL_MSC
// Disable in entire file.
#pragma warning(disable: 4503)

// Disable for includes.
#pragma warning(push)
#pragma warning(disable: 4477)
#elif MSL_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#endif

#include <boost/algorithm/string/replace.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>

#if MSL_MSC
#pragma warning(pop)
#elif MSL_CLANG
#pragma GCC diagnostic pop
#endif

#include <cstring>
#include <fstream>

namespace msl
{

namespace
{

enum class Action
{
	Error,
	Use,
	Skip
};

void handleException(Output& output, Output::Level level, const boost::wave::cpp_exception& e)
{
	std::string message = e.description();
	message = message.substr(
		std::strlen(boost::wave::util::get_severity(e.get_severity())) + 2);

	// Boost wave is an implementation detail, don't to have it be user-facing.
	boost::algorithm::replace_first(message,
		"encountered #error directive or #pragma wave stop()", "encountered #error directive");

	output.addMessage(level, e.file_name(), e.line_no(), e.column_no(), false, message);
}

void handleException(Output& output, Output::Level level,
	const boost::wave::cpplexer::cpplexer_exception& e)
{
	std::string message = e.description();
	message = message.substr(
		std::strlen(boost::wave::util::get_severity(e.get_severity())) + 2);
	output.addMessage(level, e.file_name(), e.line_no(), e.column_no(), false, message);
}

class Hooks : public boost::wave::context_policies::default_preprocessing_hooks
{
public:
	Hooks()
		: m_output(nullptr), m_error(false)
	{
	}

	void setOutput(Output& output)
	{
		m_output = &output;
	}

	bool hadError() const
	{
		return m_error;
	}

	template <typename ContextT, typename ExceptionT>
	void throw_exception(ContextT const&, ExceptionT const& e)
	{
		if (e.get_errorcode() == boost::wave::preprocess_exception::last_line_not_terminated)
			return;

		Output::Level level;
		switch (e.get_severity())
		{
			case boost::wave::util::severity_remark:
				level = Output::Level::Info;
				break;
			case boost::wave::util::severity_warning:
				level = Output::Level::Warning;
				break;
			default:
				level = Output::Level::Error;
				m_error = true;
				break;
		}

		// Only print out if not throwing an exception. The exception handler must also print out
		// due to exceptions being thrown from outside of this.
		if (e.is_recoverable())
		{
			if (m_output)
				handleException(*m_output, level, e);
		}
		else
			boost::throw_exception(e);
	}
private:
	Output* m_output;
	bool m_error;
};

template <typename FlexStr>
std::string toString(const FlexStr& str)
{
	return str.c_str();
}

Action getType(Token::Type& type, Output& output, const boost::wave::cpplexer::lex_token<>& token)
{
	const auto& position = token.get_position();
	switch (CATEGORY_FROM_TOKEN(token))
	{
		case CATEGORY_FROM_TOKEN(boost::wave::IntegerLiteralTokenType):
			type = Token::Type::IntLiteral;
			return Action::Use;
		case CATEGORY_FROM_TOKEN(boost::wave::FloatingLiteralTokenType):
			type = Token::Type::FloatLiteral;
			return Action::Use;
		case CATEGORY_FROM_TOKEN(boost::wave::BoolLiteralTokenType):
			type = Token::Type::BoolLiteral;
			return Action::Use;

		case CATEGORY_FROM_TOKEN(boost::wave::OperatorTokenType):
			type = Token::Type::Symbol;
			return Action::Use;

		case CATEGORY_FROM_TOKEN(boost::wave::StringLiteralTokenType):
		case CATEGORY_FROM_TOKEN(boost::wave::CharacterLiteralTokenType):
			output.addMessage(Output::Level::Error, toString(position.get_file()),
				position.get_line(), position.get_column(), false,
				"Invalid token '" + toString(token.get_value()) + "'");
			return Action::Error;

		case CATEGORY_FROM_TOKEN(boost::wave::PPTokenType):
		case CATEGORY_FROM_TOKEN(boost::wave::EOFTokenType):
			return Action::Skip;

		case CATEGORY_FROM_TOKEN(boost::wave::EOLTokenType):
		case CATEGORY_FROM_TOKEN(boost::wave::WhiteSpaceTokenType):
			type = Token::Type::Whitespace;
			return Action::Use;

		default:
			type = Token::Type::Identifier;
			return Action::Use;
	};
}

} // namespace

Preprocessor::Preprocessor()
	: m_supportsUniformBlocks(true)
{
}

void Preprocessor::setSupportsUniformBlocks(bool supports)
{
	m_supportsUniformBlocks = supports;
}

void Preprocessor::addIncludePath(std::string path)
{
	m_includePaths.push_back(std::move(path));
}

void Preprocessor::addDefine(std::string name, std::string value)
{
	m_defines.emplace_back(std::move(name), std::move(value));
}

bool Preprocessor::preprocess(TokenList& tokenList, Output& output,
	const std::string& fileName, const std::vector<std::string>& headerLines) const
{
	std::ifstream stream(fileName);
	if (!stream.is_open())
	{
		output.addMessage(Output::Level::Error, "", 0, 0, false,
			"could not find file: " + fileName);
		return false;
	}

	return preprocess(tokenList, output, stream, fileName, headerLines);
}

bool Preprocessor::preprocess(TokenList& tokenList, Output& output, std::istream& stream,
	const std::string& fileName, const std::vector<std::string>& headerLines) const
{
	std::string input(std::istreambuf_iterator<char>(stream.rdbuf()),
		std::istreambuf_iterator<char>());
	if (!headerLines.empty())
	{
		std::string prefix = "#line 1 \"pre-header\"\n";
		for (const std::string& line : headerLines)
		{
			prefix += line;
			prefix += '\n';
		}
		prefix += "#line 1 \"";
		prefix += boost::algorithm::replace_all_copy(fileName, "\\", "\\\\");
		prefix += "\"\n";
		input = prefix + input;
	}

	const auto language = static_cast<boost::wave::language_support>(
		boost::wave::support_c99 |
		boost::wave::support_option_convert_trigraphs |
		boost::wave::support_option_insert_whitespace |
		boost::wave::support_option_include_guard_detection);

	using LexToken = boost::wave::cpplexer::lex_token<>;
	using LexIterator = boost::wave::cpplexer::lex_iterator<LexToken>;
	using Context = boost::wave::context<std::string::iterator, LexIterator,
		boost::wave::iteration_context_policies::load_file_to_string, Hooks>;

	try
	{
		Context context(input.begin(), input.end(), fileName.c_str());
		context.get_hooks().setOutput(output);
		context.set_language(language);
		if (m_supportsUniformBlocks)
			context.add_macro_definition("INSTANCE(x)=x", true);
		else
			context.add_macro_definition("INSTANCE(x)=uniforms", true);
		for (const std::string& includePath : m_includePaths)
		{
			context.add_include_path(includePath.c_str());
			context.add_sysinclude_path(includePath.c_str());
		}
		for (const std::pair<std::string, std::string>& define : m_defines)
			context.add_macro_definition(define.first + "=" + define.second, true);

		std::vector<Token> tokens;
		for (const LexToken& token : context)
		{
			const auto& position = token.get_position();
			Token::Type type;
			switch (getType(type, output, token))
			{
				case Action::Error:
					return false;
				case Action::Skip:
					continue;
				default:
					break;
			}

			tokens.emplace_back(type, token.get_value().c_str(),
				tokenList.stringPtr(position.get_file().c_str()), position.get_line(),
				position.get_column());
		}

		tokenList.m_tokens = std::move(tokens);
		return !context.get_hooks().hadError();
	}
	catch (const boost::wave::cpp_exception& e)
	{
		handleException(output, Output::Level::Error, e);
		return false;
	}
	catch (const boost::wave::cpplexer::cpplexer_exception& e)
	{
		handleException(output, Output::Level::Error, e);
		return false;
	}
}

} // namespace msl
