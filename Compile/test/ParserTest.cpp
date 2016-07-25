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

#include "Helpers.h"
#include <MSL/Compile/Output.h>
#include "Parser.h"
#include "Preprocessor.h"
#include <gtest/gtest.h>

namespace msl
{

TEST(ParserTest, StageFilters)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output,
		(inputDir/"StageFilters.msl").string()));
	EXPECT_TRUE(parser.parse(output, "StageFilters.msl"));

	std::vector<Parser::LineMapping> lineMappings;
	EXPECT_EQ(readFile(outputDir/"StageFilters.vert"),
		parser.createShaderString(lineMappings, Parser::Stage::Vertex) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.tessc"),
		parser.createShaderString(lineMappings, Parser::Stage::TessellationControl) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.tesse"),
		parser.createShaderString(lineMappings, Parser::Stage::TessellationEvaluation) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.geom"),
		parser.createShaderString(lineMappings, Parser::Stage::Geometry) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.frag"),
		parser.createShaderString(lineMappings, Parser::Stage::Fragment) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.comp"),
		parser.createShaderString(lineMappings, Parser::Stage::Compute) + '\n');
}

} // namespace msl
