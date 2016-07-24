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
#include <algorithm>
#include <string>
#include <vector>

/**
 * @file
 * @brief Manager for the warning and error output from the compiler.
 */

namespace msl
{

/**
 * @brief Class to hold a list of warnings and errors output from the compiler.
 */
class Output
{
public:
	/**
	 * @brief Level of a message.
	 */
	enum class Level
	{
		Info,    ///< Info message that doesn't indicate an error.
		Warning, ///< Warning that could be an issue, but doesn't stop compilation.
		Error    ///< Error that halts execution of the compiler.
	};

	/**
	 * @brief Structure for an individual message.
	 */
	struct Message
	{
		/**
		 * @brief Default constructor.
		 */
		inline Message();

		/**
		 * @brief Constructs the message with each parameter.
		 * @param level_ The level of the message.
		 * @param file_ The file that the message occurred in.
		 * @param line_ The line number of the error.
		 * @param column_ The column of the error within the line.
		 * @param continued_ Whether or not the message is a continuation of the last message.
		 * @param message_ The string message.
		 */
		inline Message(Level level_, std::string file_, std::size_t line_, std::size_t column_,
			bool continued_, std::string message_);

		/**
		 * @brief The level of the message.
		 */
		Level level;

		/**
		 * @brief The file that the message occurred in.
		 */
		std::string file;

		/**
		 * @brief The line number of the error.
		 */
		std::size_t line;

		/**
		 * @brief The column of the error within the line.
		 */
		std::size_t column;

		/**
		 * @brief Whether or not the message is a continuation of the last message.
		 *
		 * Continued messages won't count toward the total number of warnings and errors.
		 */
		bool continued;

		/**
		 * @brief The string message.
		 */
		std::string message;
	};

	/**
	 * @brief Gets the list of messages to output.
	 */
	inline const std::vector<Message>& getMessages() const;

	/**
	 * @brief Returns wither or not the list of messages is empty.
	 * @return True if no messages.
	 */
	inline bool empty() const;

	/**
	 * @brief Gets the number of warnings.
	 * @return The number of warnings. Continued messages aren't counted.
	 */
	inline std::size_t getWarningCount() const;

	/**
	 * @brief Gets the number of errors.
	 * @return The number of errors. Continued messages aren't counted.
	 */
	inline std::size_t getErrorCount() const;

	/**
	 * @brief Adds a message to the output.
	 * @param message The message to add.
	 */
	inline void addMessage(Message message);

	/**
	 * @brief Adds a message to the output.
	 * @param level The level of the message.
	 * @param file The file that the message occurred in.
	 * @param line The line number of the error.
	 * @param column The column of the error within the line.
	 * @param continued Whether or not the message is a continuation of the last message.
	 * @param message The string message.
	 */
	inline void addMessage(Level level, std::string file, std::size_t line, std::size_t column,
		bool continued, std::string message);

	/**
	 * @brief Clears the list of messages.
	 */
	inline void clear();

private:
	std::vector<Message> m_messages;
};

inline Output::Message::Message()
	: level(Level::Warning)
	, line(0)
	, column(0)
	, continued(false)
{
}

inline Output::Message::Message(Level level_, std::string file_, std::size_t line_,
	std::size_t column_, bool continued_, std::string message_)
	: level(level_)
	, file(std::move(file_))
	, line(line_)
	, column(column_)
	, continued(continued_)
	, message(message_)

{
}

inline const std::vector<Output::Message>& Output::getMessages() const
{
	return m_messages;
}

inline bool Output::empty() const
{
	return m_messages.empty();
}

inline std::size_t Output::getWarningCount() const
{
	std::size_t count = 0;
	for (const Message& message : m_messages)
	{
		if (message.level == Level::Warning && !message.continued)
			++count;
	}
	return count;
}

inline std::size_t Output::getErrorCount() const
{
	std::size_t count = 0;
	for (const Message& message : m_messages)
	{
		if (message.level == Level::Error && !message.continued)
			++count;
	}
	return count;
}

inline void Output::addMessage(Message message)
{
	m_messages.push_back(std::move(message));
}

inline void Output::addMessage(Level level, std::string file, std::size_t line,
	std::size_t column, bool continued, std::string message)
{
	m_messages.emplace_back(level, std::move(file), line, column, continued, std::move(message));
}

inline void Output::clear()
{
	m_messages.clear();
}

} // namespace msl
