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
#include <cstring>
#include <sstream>

namespace msl
{

bool operator==(const Parser::LineMapping& map1, const Parser::LineMapping& map2)
{
	return std::strcmp(map1.fileName, map2.fileName) == 0 && map1.line == map2.line;
}

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

	Parser::Pipeline pipeline = {};
	std::vector<Parser::LineMapping> lineMappings;
	EXPECT_EQ(readFile(outputDir/"StageFilters.vert"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::Vertex) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.tessc"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::TessellationControl) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.tesse"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::TessellationEvaluation) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.geom"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::Geometry) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.frag"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::Fragment) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.comp"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::Compute) + '\n');
}

TEST(ParserTest, InvalidStageName)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("[[asdf]] int bla;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(3U, output.getMessages()[0].column);
	EXPECT_EQ("unknown stage type: asdf", output.getMessages()[0].message);
}

TEST(ParserTest, StageDeclNotFirst)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("int [[fragment]] bla;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(6U, output.getMessages()[0].column);
	EXPECT_EQ("stage declaration must be at the start of an element", output.getMessages()[0].message);
}

TEST(ParserTest, StageDeclInvalidChar)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("[[[fragment]] int bla;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(3U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected token: [", output.getMessages()[0].message);
}

TEST(ParserTest, UnterminatedEnd)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("[[fragment]] int bla; float foo");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(29U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected end of file", output.getMessages()[0].message);
}

TEST(ParserTest, ExtraEndParen)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("int foo()) {gl_position = bar[2];}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(10U, output.getMessages()[0].column);
	EXPECT_EQ("encountered ) without opening (", output.getMessages()[0].message);
}

TEST(ParserTest, MissingCloseParen)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("int foo( {gl_position = bar[2];}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(2U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(32U, output.getMessages()[0].column);
	EXPECT_EQ("reached end of file without terminating )", output.getMessages()[0].message);

	EXPECT_EQ(path, output.getMessages()[1].file);
	EXPECT_EQ(1U, output.getMessages()[1].line);
	EXPECT_EQ(8U, output.getMessages()[1].column);
	EXPECT_EQ("see opening (", output.getMessages()[1].message);
}

TEST(ParserTest, ExtraEndBrace)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("int foo() {gl_position = bar[2];}}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(34U, output.getMessages()[0].column);
	EXPECT_EQ("encountered } without opening {", output.getMessages()[0].message);
}

TEST(ParserTest, MissingCloseBrace)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("int foo() {gl_position = bar[2];");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(2U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(32U, output.getMessages()[0].column);
	EXPECT_EQ("reached end of file without terminating }", output.getMessages()[0].message);

	EXPECT_EQ(path, output.getMessages()[1].file);
	EXPECT_EQ(1U, output.getMessages()[1].line);
	EXPECT_EQ(11U, output.getMessages()[1].column);
	EXPECT_EQ("see opening {", output.getMessages()[1].message);
}

TEST(ParserTest, SquareEndBrace)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("int foo() {gl_position = bar[2]];}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(32U, output.getMessages()[0].column);
	EXPECT_EQ("encountered ] without opening [", output.getMessages()[0].message);
}

TEST(ParserTest, MissingCloseSquare)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("int foo() {gl_position = bar[2;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(2U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(32U, output.getMessages()[0].column);
	EXPECT_FALSE(output.getMessages()[0].continued);
	EXPECT_EQ("reached end of file without terminating ]", output.getMessages()[0].message);

	EXPECT_EQ(path, output.getMessages()[1].file);
	EXPECT_EQ(1U, output.getMessages()[1].line);
	EXPECT_EQ(29U, output.getMessages()[1].column);
	EXPECT_TRUE(output.getMessages()[1].continued);
	EXPECT_EQ("see opening [", output.getMessages()[1].message);
}

TEST(ParserTest, Pipeline)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output,
		(inputDir/"Pipeline.msl").string()));
	EXPECT_TRUE(parser.parse(output, "Pipeline.msl"));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	EXPECT_EQ("Foo", pipeline.name);
	EXPECT_EQ("vertEntry", pipeline.entryPoints[0]);
	EXPECT_EQ("tessControlEntry", pipeline.entryPoints[1]);
	EXPECT_EQ("tessEvaluationEntry", pipeline.entryPoints[2]);
	EXPECT_EQ("geometryEntry", pipeline.entryPoints[3]);
	EXPECT_EQ("fragEntry", pipeline.entryPoints[4]);
	EXPECT_EQ("computeEntry", pipeline.entryPoints[5]);

	std::vector<Parser::LineMapping> lineMappings;
	EXPECT_EQ(readFile(outputDir/"Pipeline.frag"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::Fragment) + '\n');
}

TEST(ParserTest, UnnamedPipeline)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline {compute = computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(10U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected token: {", output.getMessages()[0].message);
}

TEST(ParserTest, PipelineMissingOpenBrace)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline Test compute = computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(15U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected token: compute", output.getMessages()[0].message);
}

TEST(ParserTest, PipelineUnknownStage)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline Test {asdf = computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(16U, output.getMessages()[0].column);
	EXPECT_EQ("unknown stage type: asdf", output.getMessages()[0].message);
}

TEST(ParserTest, PipelineMissingEquals)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline Test {compute computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(24U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected token: computeEntry", output.getMessages()[0].message);
}

TEST(ParserTest, PipelineMissingEntryPoint)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline Test {compute =;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(25U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected token: ;", output.getMessages()[0].message);
}

TEST(ParserTest, PipelineMissingSemicolon)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline Test {compute = computeEntry}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(38U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected token: }", output.getMessages()[0].message);
}

TEST(ParserTest, PipelineMissingEndBrace)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline Test {compute = computeEntry;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(38U, output.getMessages()[0].column);
	EXPECT_EQ("unexpected end of file", output.getMessages()[0].message);
}

TEST(ParserTest, DuplicatePipeline)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("pipeline Test {} pipeline Test{}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl"));

	ASSERT_EQ(2U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(27U, output.getMessages()[0].column);
	EXPECT_FALSE(output.getMessages()[0].continued);
	EXPECT_EQ("pipeline of name Test already declared", output.getMessages()[0].message);

	EXPECT_EQ(path, output.getMessages()[1].file);
	EXPECT_EQ(1U, output.getMessages()[1].line);
	EXPECT_EQ(10U, output.getMessages()[1].column);
	EXPECT_TRUE(output.getMessages()[1].continued);
	EXPECT_EQ("see other declaration of pipeline Test", output.getMessages()[1].message);
}

TEST(ParserTest, RemoveUniformBlocks)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output,
		(inputDir/"RemoveUniformBlocks.msl").string()));
	EXPECT_TRUE(parser.parse(output, "RemoveUniformBlocks.msl", Parser::RemoveUniformBlocks));

	std::vector<Parser::LineMapping> lineMappings;
	Parser::Pipeline pipeline;
	EXPECT_EQ(readFile(outputDir/"RemoveUniformBlocks.vert"),
		parser.createShaderString(lineMappings, pipeline, Parser::Stage::Vertex));
}

TEST(ParserTest, RemoveNamedUniformBlock)
{
	std::string path = (exeDir/"test.msl").string();
	std::stringstream stream("uniform Test {vec4 test;} testBlock;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output, "test.msl", Parser::RemoveUniformBlocks));

	ASSERT_EQ(1U, output.getMessages().size());
	EXPECT_EQ(path, output.getMessages()[0].file);
	EXPECT_EQ(1U, output.getMessages()[0].line);
	EXPECT_EQ(27U, output.getMessages()[0].column);
	EXPECT_EQ("can not have a uniform block instance name when removing uniform blocks",
		output.getMessages()[0].message);
}

TEST(ParserTest, LineNumbers)
{
	boost::filesystem::path inputDir = exeDir/"inputs";

	std::string fileName = (inputDir/"LineNumbers.msl").string();
	std::string includeFileName = (inputDir/"LineNumbers.mslh").string();

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(inputDir.string());
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, fileName));
	EXPECT_TRUE(parser.parse(output, fileName));

	std::vector<Parser::LineMapping> expectedMappings =
	{
		Parser::LineMapping{fileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 3},
		Parser::LineMapping{includeFileName.c_str(), 4},
		Parser::LineMapping{includeFileName.c_str(), 5},
		Parser::LineMapping{includeFileName.c_str(), 6},
		Parser::LineMapping{fileName.c_str(), 6},
		Parser::LineMapping{fileName.c_str(), 7},
		Parser::LineMapping{fileName.c_str(), 8},
		Parser::LineMapping{fileName.c_str(), 9},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 18},
		Parser::LineMapping{fileName.c_str(), 19},
		Parser::LineMapping{fileName.c_str(), 20},
		Parser::LineMapping{fileName.c_str(), 21}
	};

	std::vector<Parser::LineMapping> lineMappings;
	Parser::Pipeline pipeline;
	parser.createShaderString(lineMappings, pipeline, Parser::Stage::Vertex);
	EXPECT_EQ(expectedMappings, lineMappings);
}

TEST(ParserTest, LineNumbersRemoveUniformBlocks)
{
	boost::filesystem::path inputDir = exeDir/"inputs";

	std::string fileName = (inputDir/"LineNumbers.msl").string();
	std::string includeFileName = (inputDir/"LineNumbers.mslh").string();

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(inputDir.string());
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, fileName));
	EXPECT_TRUE(parser.parse(output, fileName, Parser::RemoveUniformBlocks));

	std::vector<Parser::LineMapping> expectedMappings =
	{
		Parser::LineMapping{fileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 5},
		Parser::LineMapping{fileName.c_str(), 6},
		Parser::LineMapping{fileName.c_str(), 7},
		Parser::LineMapping{fileName.c_str(), 8},
		Parser::LineMapping{fileName.c_str(), 9},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 18},
		Parser::LineMapping{fileName.c_str(), 19},
		Parser::LineMapping{fileName.c_str(), 20},
		Parser::LineMapping{fileName.c_str(), 21}
	};

	std::vector<Parser::LineMapping> lineMappings;
	Parser::Pipeline pipeline;
	parser.createShaderString(lineMappings, pipeline, Parser::Stage::Vertex);
	EXPECT_EQ(expectedMappings, lineMappings);
}

} // namespace msl
