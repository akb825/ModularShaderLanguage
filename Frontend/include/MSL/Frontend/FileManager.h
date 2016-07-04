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
#include <MSL/Frontend/Token.h>
#include <deque>
#include <istream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @file
 * @brief Manager for loading files.
 */

namespace msl
{

/**
 * @brief Base class for a manager for loading files.
 *
 * This contains the basic behavior of loading and tokenizing files, while base classes will provide
 * the logic for loading the file into a string.
 */
class MSL_FRONTEND_EXPORT FileManager
{
public:
	/**
	 * @brief Constant for an invalid index.
	 */
	static const std::size_t invalidIndex = (std::size_t)-1;

	/**
	 * @brief Structure defining the information within a file.
	 */
	struct File
	{
		/**
		 * @brief The file name of the file.
		 *
		 * This is what was passed into loadFile().
		 */
		std::string fileName;

		/**
		 * @brief The full path of the file.
		 *
		 * This is the file name combined with the include path. This isn't necessarily an absoulte
		 * path.
		 */
		std::string fullPath;

		/**
		 * @brief The contents of the file.
		 */
		std::string contents;

		/**
		 * @brief The tokens from the file contents.
		 */
		std::vector<Token> tokens;
	};

	/**
	 * @brief Default constructor.
	 */
	inline FileManager() {}
	inline virtual ~FileManager() {}

	FileManager(const FileManager&) = delete;
	FileManager& operator=(const FileManager&) = delete;

	/**
	 * @brief Gets the list of include paths.
	 * @return The include paths.
	 */
	inline std::deque<std::string>& getIncludePaths();

	/** @copydoc getIncludePaths() */
	inline const std::deque<std::string>& getIncludePaths() const;

	/**
	 * @brief Sets the include paths.
	 * @param includePaths The include paths.
	 */
	inline void setIncludePaths(std::deque<std::string> includePaths);

	/**
	 * @brief Gets the number of files loaded.
	 */
	inline std::size_t getFileCount() const;

	/**
	 * @brief Gets a file by index.
	 * @param index The index of the file.
	 * @return The file or nullptr if the index is invalid.
	 */
	inline std::shared_ptr<const File> getFile(std::size_t index) const;

	/**
	 * @brief Gets the index of a file.
	 * @param file The file to get the index of.
	 * @return The index of the file, or invalidIndex if not found.
	 */
	inline std::size_t getFileIndex(const File& file) const;

	/**
	 * @brief Loads a file.
	 *
	 * If the file has already been loaded, this will return the previously loaded file.
	 *
	 * If fileName is an absolute path, ignoreIncludePaths is true, or the include path list is
	 * empty, fileName will be loaded as-is. Otherwise, it will append fileName to each include path
	 * in order to load the file.
	 *
	 * @param fileName The name of the file.
	 * @param ignoreIncludePaths True to load the file as-is without using the include paths.
	 * @return The loaded file or nullptr if it couldn't be loaded.
	 */
	std::shared_ptr<const File> loadFile(const std::string& fileName,
		bool ignoreIncludePaths = false);

	/**
	 * @brief Clears the files.
	 */
	inline void clearFiles();

	/**
	 * @brief Clears this to the default state.
	 */
	inline void clear();

protected:
	/**
	 * @brief Helper function for reading all the contents from a stream.
	 */
	static std::string readFromStream(std::istream& stream);

	/**
	 * @brief Loads the contents of a file.
	 *
	 * This may be called multiple times each file load based on the entries on the list of include
	 * paths.
	 *
	 * @param[out] contents The contents of the file.
	 * @param fileName The name of the file to load.
	 * @return False if the file couldn't be loaded.
	 */
	virtual bool loadFileContents(std::string& contents, const std::string& fileName) = 0;

	/**
	 * @brief Function called when the files are cleared.
	 */
	virtual void filesCleared() {}

private:
	std::deque<std::string> m_includePaths;
	std::vector<std::shared_ptr<File>> m_files;
	std::unordered_map<std::string, std::size_t> m_fileMap;
};

inline std::deque<std::string>& FileManager::getIncludePaths()
{
	return m_includePaths;
}

inline const std::deque<std::string>& FileManager::getIncludePaths() const
{
	return m_includePaths;
}

inline void FileManager::setIncludePaths(std::deque<std::string> includePaths)
{
	m_includePaths = std::move(includePaths);
}

inline std::size_t FileManager::getFileCount() const
{
	return m_files.size();
}

inline std::shared_ptr<const FileManager::File> FileManager::getFile(std::size_t index) const
{
	if (index >= m_files.size())
		return nullptr;

	return m_files[index];
}

inline std::size_t FileManager::getFileIndex(const File& file) const
{
	auto foundIter = m_fileMap.find(file.fullPath);
	if (foundIter == m_fileMap.end())
		return invalidIndex;

	return foundIter->second;
}

inline void FileManager::clearFiles()
{
	m_files.clear();
	m_fileMap.clear();
	filesCleared();
}

inline void FileManager::clear()
{
	clearFiles();
	m_includePaths.clear();
}

} // namespace msl
