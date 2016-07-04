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

#include <MSL/Frontend/Path.h>
#include <gtest/gtest.h>

namespace msl
{

#if MSL_WINDOWS
static const char* absolutePath = "\\absolute\\path";
static const char* path1 = "path\\one";
static const char* path2 = "second\\path";
static const char* combinedPath = "path\\one\\second\\path";
static const char* directoryPath = "directory\\path\\";
static const char* directoryPathTrimmed = "directory\\path";
static const char* combinedDirectoryPath = "directory\\path\\second\\path";
#else
static const char* absolutePath = "/absolute/path";
static const char* path1 = "path/one";
static const char* path2 = "second/path";
static const char* combinedPath = "path/one/second/path";
static const char* directoryPath = "directory/path/";
static const char* directoryPathTrimmed = "directory/path";
static const char* combinedDirectoryPath = "directory/path/second/path";
#endif

TEST(PathTest, IsAbsolute)
{
	EXPECT_TRUE(Path::isAbsolute(absolutePath));
	EXPECT_FALSE(Path::isAbsolute(path1));
#if MSL_WINDOWS
	EXPECT_FALSE(Path::isAbsolute("C:\\absolute\\path"));
#endif
}

TEST(PathTest, Combine)
{
	EXPECT_EQ("", Path::combine("", ""));
	EXPECT_EQ(path1, Path::combine(path1, ""));
	EXPECT_EQ(path2, Path::combine("", path2));
	EXPECT_EQ(combinedPath, Path::combine(path1, path2));
	EXPECT_EQ(combinedDirectoryPath, Path::combine(directoryPath, path2));
}

TEST(PathTest, GetDirectory)
{
	EXPECT_EQ("", Path::getDirectory(""));
	EXPECT_EQ("second", Path::getDirectory(path2));
	EXPECT_EQ(directoryPathTrimmed, Path::getDirectory(directoryPath));
	EXPECT_EQ("", Path::getDirectory("file"));
}

TEST(PathTest, GetFile)
{
	EXPECT_EQ("", Path::getFile(""));
	EXPECT_EQ("one", Path::getFile(path1));
	EXPECT_EQ("", Path::getFile(directoryPath));
	EXPECT_EQ("file", Path::getFile("file"));
}

} // namespace msl
