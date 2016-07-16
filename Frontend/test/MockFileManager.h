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

#include <MSL/Frontend/FileManager.h>
#include <MSL/Frontend/Path.h>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace msl
{

class MockFileManager : public FileManager
{
public:
	void addFile(const std::string& fileName, std::string contents)
	{
		m_files.emplace(Path::normalize(fileName), std::stringstream(std::move(contents)));
	}

	const std::vector<std::string>& getRequestedFiles() const
	{
		return m_requestedFiles;
	}


	bool loadFileContents(std::string& contents, const std::string& fileName) override
	{
		m_requestedFiles.push_back(contents);
		auto foundIter = m_files.find(fileName);
		if (foundIter == m_files.end())
			return false;

		contents = readFromStream(foundIter->second);
		return true;
	}

	void filesCleared() override
	{
		m_files.clear();
		m_requestedFiles.clear();
	}

private:
	std::unordered_map<std::string, std::stringstream> m_files;
	std::vector<std::string> m_requestedFiles;
};

} // namespace msl
