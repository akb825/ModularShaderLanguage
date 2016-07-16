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

#include "MockFileManager.h"
#include "TokenHelpers.h"
#include <MSL/Frontend/Path.h>
#include <gtest/gtest.h>

namespace msl
{

TEST(FileManagerTest, IncludePaths)
{
	MockFileManager fileManager;
	const MockFileManager& constFileManager = fileManager;

	std::deque<std::string> expectedPaths;
	expectedPaths.push_back("test");

	fileManager.getIncludePaths().push_back("test");
	EXPECT_EQ(expectedPaths, fileManager.getIncludePaths());
	EXPECT_EQ(fileManager.getIncludePaths(), constFileManager.getIncludePaths());

	expectedPaths.push_front("asdf");
	fileManager.setIncludePaths(expectedPaths);
	EXPECT_EQ(expectedPaths, fileManager.getIncludePaths());

	fileManager.clearFiles();
	EXPECT_EQ(expectedPaths, fileManager.getIncludePaths());
	fileManager.clear();
	EXPECT_TRUE(fileManager.getIncludePaths().empty());
}

TEST(FileManagerTest, ReadFromStream)
{
	std::string contents = "This will be a very long string.\n";
	for (unsigned int i = 0; i < 4; ++i)
		contents += contents;

	MockFileManager fileManager;
	fileManager.addFile("test", contents);

	std::string readContents;
	EXPECT_FALSE(fileManager.loadFileContents(readContents, "asdf"));
	EXPECT_TRUE(fileManager.loadFileContents(readContents, "test"));
	EXPECT_EQ(contents, readContents);
}

TEST(FileManagerTest, LoadFile)
{
	MockFileManager fileManager;
	fileManager.getIncludePaths().push_back("test");
	fileManager.getIncludePaths().push_back("test/c/");
	fileManager.getIncludePaths().push_back("/");

	std::string fileA = "int a = 3;";
	fileManager.addFile("test/b/../a/.//fileA", fileA);
	std::vector<Token> expectedTokensA;
	addToken(expectedTokensA, fileA, Token::Type::Int, "int", 0);
	addToken(expectedTokensA, fileA, Token::Type::Whitespace, " ", 0);
	addToken(expectedTokensA, fileA, Token::Type::Identifier, "a", 0);
	addToken(expectedTokensA, fileA, Token::Type::Whitespace, " ", 0);
	addToken(expectedTokensA, fileA, Token::Type::Equal, "=", 0);
	addToken(expectedTokensA, fileA, Token::Type::Whitespace, " ", 0);
	addToken(expectedTokensA, fileA, Token::Type::IntLiteral, "3", 0);
	addToken(expectedTokensA, fileA, Token::Type::Semicolon, ";", 0);

	std::string fileB = "float b = 3.14;";
	fileManager.addFile("/b/fileB", fileB);
	std::vector<Token> expectedTokensB;
	addToken(expectedTokensB, fileB, Token::Type::Float, "float", 1);
	addToken(expectedTokensB, fileB, Token::Type::Whitespace, " ", 1);
	addToken(expectedTokensB, fileB, Token::Type::Identifier, "b", 1);
	addToken(expectedTokensB, fileB, Token::Type::Whitespace, " ", 1);
	addToken(expectedTokensB, fileB, Token::Type::Equal, "=", 1);
	addToken(expectedTokensB, fileB, Token::Type::Whitespace, " ", 1);
	addToken(expectedTokensB, fileB, Token::Type::FloatLiteral, "3.14", 1);
	addToken(expectedTokensB, fileB, Token::Type::Semicolon, ";", 1);

	std::shared_ptr<const FileManager::File> loadedFileA = fileManager.loadFile("a/fileA");
	ASSERT_NE(nullptr, loadedFileA);
	EXPECT_EQ(Path::normalize("test/a/fileA"), loadedFileA->path);
	EXPECT_EQ(fileA, loadedFileA->contents);
	EXPECT_EQ(expectedTokensA, loadedFileA->tokens);
	EXPECT_EQ(1U, fileManager.getFileCount());
	EXPECT_EQ(0U, fileManager.getFileIndex(*loadedFileA));
	EXPECT_EQ(loadedFileA, fileManager.getFile(0));

	std::shared_ptr<const FileManager::File> loadedFileB = fileManager.loadFile("b/fileB");
	ASSERT_NE(nullptr, loadedFileB);
	EXPECT_EQ(Path::normalize("/b/fileB"), loadedFileB->path);
	EXPECT_EQ(fileB, loadedFileB->contents);
	EXPECT_EQ(expectedTokensB, loadedFileB->tokens);
	EXPECT_EQ(2U, fileManager.getFileCount());
	EXPECT_EQ(1U, fileManager.getFileIndex(*loadedFileB));
	EXPECT_EQ(loadedFileB, fileManager.getFile(1));

	EXPECT_EQ(loadedFileA, fileManager.loadFile("../a/fileA"));
	EXPECT_EQ(loadedFileA, fileManager.loadFile("b/c/../..//a/././/fileA"));
	EXPECT_EQ(loadedFileB, fileManager.loadFile("/b//fileB"));
	EXPECT_EQ(nullptr, fileManager.loadFile("a/fileA", true));
	EXPECT_EQ(nullptr, fileManager.loadFile("b/fileB", true));

	EXPECT_EQ(FileManager::invalidIndex, fileManager.getFileIndex(FileManager::File()));
	EXPECT_EQ(nullptr, fileManager.getFile(fileManager.getFileCount()));

	fileManager.clear();
	EXPECT_EQ(0U, fileManager.getFileCount());
}

} // namespace msl
