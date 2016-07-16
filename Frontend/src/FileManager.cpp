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

#include <MSL/Frontend/FileManager.h>
#include <MSL/Frontend/Parse/Lexer.h>
#include <MSL/Frontend/Path.h>
#include <cstring>

namespace msl
{

const std::size_t FileManager::invalidIndex;

std::string FileManager::readFromStream(std::istream &stream)
{
	const unsigned int bufferSize = 1024;
	char buffer[bufferSize];
	std::string retVal;
	std::size_t pos = 0;
	while (stream)
	{
		stream.read(buffer, bufferSize);
		std::size_t readCount = static_cast<std::size_t>(stream.gcount());
		retVal.resize(pos + readCount);
		std::memcpy(&retVal[pos], buffer, readCount);
		pos += readCount;
	}

	return retVal;
}

std::shared_ptr<const FileManager::File> FileManager::loadFile(const std::string& fileName,
	bool ignoreIncludePaths)
{
	std::string fullPath;
	std::string contents;
	if (ignoreIncludePaths || m_includePaths.empty() || Path::isAbsolute(fileName))
	{
		std::string normalizedFileName = Path::normalize(fileName);
		auto foundIter = m_fileMap.find(normalizedFileName);
		if (foundIter != m_fileMap.end())
			return m_files[foundIter->second];

		if (!loadFileContents(contents, normalizedFileName))
			return nullptr;

		fullPath = normalizedFileName;
	}
	else
	{
		for (const std::string& includePath : m_includePaths)
		{
			std::string combinedPath = Path::normalize(Path::combine(includePath, fileName));
			auto foundIter = m_fileMap.find(combinedPath);
			if (foundIter != m_fileMap.end())
				return m_files[foundIter->second];

			if (loadFileContents(contents, combinedPath))
			{
				fullPath = std::move(combinedPath);
				break;
			}
		}

		if (fullPath.empty())
			return nullptr;
	}

	std::size_t fileIndex = m_files.size();
	std::shared_ptr<File> file = std::make_shared<File>();
	file->path = fullPath;
	file->contents = std::move(contents);
	file->tokens = Lexer::tokenize(fileIndex, file->contents);

	m_files.push_back(file);
	m_fileMap.emplace(fullPath, fileIndex);
	return file;
}

} // namespace msl
