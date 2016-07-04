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
#include <MSL/Frontend/FileManager.h>

/**
 * @file
 * @brief Default file manager for loading from the filesystem.
 */

namespace msl
{

/**
 * @brief Default FileManager implementation that loads directly from the filesystem.
 */
class MSL_FRONTEND_EXPORT DefaultFileManager : public FileManager
{
public:
	/**
	 * @brief Default constructor.
	 */
	DefaultFileManager() {}
	inline virtual ~DefaultFileManager() {}

	DefaultFileManager(const DefaultFileManager&) = delete;
	DefaultFileManager& operator=(const DefaultFileManager&) = delete;

protected:
	bool loadFileContents(std::string& contents, const std::string& fileName) override;
};

} // namespace msl
