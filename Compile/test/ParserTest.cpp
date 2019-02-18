/*
 * Copyright 2016-2019 Aaron Barany
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
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>

namespace msl
{

bool operator==(const Parser::LineMapping& map1, const Parser::LineMapping& map2)
{
	return pathStr(map1.fileName) == pathStr(map2.fileName) && map1.line == map2.line;
}

TEST(ParserTest, StageFilters)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output,
		pathStr(inputDir/"StageFilters.msl")));
	EXPECT_TRUE(parser.parse(output));

	Parser::Pipeline pipeline;
	std::vector<Parser::LineMapping> lineMappings;
	EXPECT_EQ(readFile(outputDir/"StageFilters.vert"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Vertex, true) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.tessc"),
		parser.createShaderString(lineMappings, output, pipeline,
			Stage::TessellationControl, true) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.tesse"),
		parser.createShaderString(lineMappings, output, pipeline,
			Stage::TessellationEvaluation, true) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.geom"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Geometry, true) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.frag"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Fragment, true) + '\n');
	EXPECT_EQ(readFile(outputDir/"StageFilters.comp"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Compute, true) + '\n');
}

TEST(ParserTest, InvalidStageName)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("[[asdf]] int bla;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(3U, messages[0].column);
	EXPECT_EQ("unknown stage type: 'asdf'", messages[0].message);
}

TEST(ParserTest, StageDeclNotFirst)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("int [[fragment]] bla;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(6U, messages[0].column);
	EXPECT_EQ("stage declaration must be at the start of an element", messages[0].message);
}

TEST(ParserTest, StageDeclInvalidChar)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("[[[fragment]] int bla;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(3U, messages[0].column);
	EXPECT_EQ("unexpected token: '['", messages[0].message);
}

TEST(ParserTest, UnterminatedEnd)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("[[fragment]] int bla; float foo");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(29U, messages[0].column);
	EXPECT_EQ("unexpected end of file", messages[0].message);
}

TEST(ParserTest, ExtraEndParen)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("int foo()) {gl_position = bar[2];}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_EQ("encountered ')' without opening '('", messages[0].message);
}

TEST(ParserTest, MissingCloseParen)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("int foo( {gl_position = bar[2];}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(2U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(32U, messages[0].column);
	EXPECT_EQ("reached end of file without terminating ')'", messages[0].message);

	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[1].file), path));
	EXPECT_EQ(1U, messages[1].line);
	EXPECT_EQ(8U, messages[1].column);
	EXPECT_EQ("see opening '('", messages[1].message);
}

TEST(ParserTest, ExtraEndBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("int foo() {gl_position = bar[2];}}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(34U, messages[0].column);
	EXPECT_EQ("encountered '}' without opening '{'", messages[0].message);
}

TEST(ParserTest, MissingCloseBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("int foo() {gl_position = bar[2];");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(2U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(32U, messages[0].column);
	EXPECT_EQ("reached end of file without terminating '}'", messages[0].message);

	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[1].file), path));
	EXPECT_EQ(1U, messages[1].line);
	EXPECT_EQ(11U, messages[1].column);
	EXPECT_EQ("see opening '{'", messages[1].message);
}

TEST(ParserTest, SquareEndBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("int foo() {gl_position = bar[2]];}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(32U, messages[0].column);
	EXPECT_EQ("encountered ']' without opening '['", messages[0].message);
}

TEST(ParserTest, MissingCloseSquare)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("int foo() {gl_position = bar[2;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(2U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(32U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("reached end of file without terminating ']'", messages[0].message);

	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[1].file), path));
	EXPECT_EQ(1U, messages[1].line);
	EXPECT_EQ(29U, messages[1].column);
	EXPECT_TRUE(messages[1].continued);
	EXPECT_EQ("see opening '['", messages[1].message);
}

TEST(ParserTest, Pipeline)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output,
		pathStr(inputDir/"Pipeline.msl")));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	EXPECT_EQ("Foo", pipeline.name);
	EXPECT_EQ("vertEntry", pipeline.entryPoints[0].value);
	EXPECT_EQ("tessControlEntry", pipeline.entryPoints[1].value);
	EXPECT_EQ("tessEvaluationEntry", pipeline.entryPoints[2].value);
	EXPECT_EQ("geometryEntry", pipeline.entryPoints[3].value);
	EXPECT_EQ("fragEntry", pipeline.entryPoints[4].value);
	EXPECT_EQ("computeEntry", pipeline.entryPoints[5].value);

	std::vector<Parser::LineMapping> lineMappings;
	EXPECT_EQ(readFile(outputDir/"Pipeline.frag"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Fragment) + '\n');
}

TEST(ParserTest, UnnamedPipeline)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline {compute = computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_EQ("unexpected token: '{', expected identifier", messages[0].message);
}

TEST(ParserTest, PipelineMissingOpenBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline Test compute = computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(15U, messages[0].column);
	EXPECT_EQ("unexpected token: 'compute', expected '{'", messages[0].message);
}

TEST(ParserTest, PipelineUnknownStage)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline Test {asdf = computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(16U, messages[0].column);
	EXPECT_EQ("unknown pipeline stage or render state name: 'asdf'", messages[0].message);
}

TEST(ParserTest, PipelineMissingEquals)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline Test {compute computeEntry;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(24U, messages[0].column);
	EXPECT_EQ("unexpected token: 'computeEntry', expected '='", messages[0].message);
}

TEST(ParserTest, PipelineMissingEntryPoint)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline Test {compute =;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(25U, messages[0].column);
	EXPECT_EQ("unexpected token: ';'", messages[0].message);
}

TEST(ParserTest, PipelineMissingSemicolon)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline Test {compute = computeEntry}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(38U, messages[0].column);
	EXPECT_EQ("unexpected token: '}'", messages[0].message);
}

TEST(ParserTest, PipelineMissingEndBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline Test {compute = computeEntry;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(38U, messages[0].column);
	EXPECT_EQ("unexpected end of file", messages[0].message);
}

TEST(ParserTest, DuplicatePipeline)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("pipeline Test {} pipeline Test{}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(2U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(27U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("pipeline of name 'Test' already declared", messages[0].message);

	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[1].file), path));
	EXPECT_EQ(1U, messages[1].line);
	EXPECT_EQ(10U, messages[1].column);
	EXPECT_TRUE(messages[1].continued);
	EXPECT_EQ("see other declaration of pipeline 'Test'", messages[1].message);
}

TEST(ParserTest, Varying)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output,
		pathStr(inputDir/"Varying.msl")));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	EXPECT_EQ("Foo", pipeline.name);
	EXPECT_EQ("vertEntry", pipeline.entryPoints[0].value);
	EXPECT_EQ("fragEntry", pipeline.entryPoints[4].value);

	std::vector<Parser::LineMapping> lineMappings;
	EXPECT_EQ(readFile(outputDir/"Varying.vert"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Vertex) + '\n');

	EXPECT_EQ(readFile(outputDir/"Varying.frag"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Fragment) + '\n');
}

TEST(ParserTest, VaryingMissingOpenParen)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying vertex, fragment) {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(9U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unexpected token: 'vertex', expected '('", messages[0].message);
}

TEST(ParserTest, VaryingInvalidOutputStage)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (asdf, fragment) {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unknown stage type: 'asdf'", messages[0].message);
}

TEST(ParserTest, VaryingMissingComma)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (vertex fragment) {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(17U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unexpected token: 'fragment', expected ','", messages[0].message);
}

TEST(ParserTest, VaryingInvalidInputStage)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (vertex, asdf) {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(18U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unknown stage type: 'asdf'", messages[0].message);
}

TEST(ParserTest, VaryingMissingCloseParen)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (vertex, fragment {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(27U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unexpected token: '{', expected ')'", messages[0].message);
}

TEST(ParserTest, VaryingMissingOpenBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (vertex, fragment) }");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(28U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unexpected token: '}', expected '{'", messages[0].message);
}

TEST(ParserTest, VaryingMissingSemicolon)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (vertex, fragment) {vec2 foo}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(37U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unexpected token: '}', expected ';'", messages[0].message);
}

TEST(ParserTest, VaryingMissingCloseBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (vertex, fragment) {");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(28U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("unexpected end of file", messages[0].message);
}

TEST(ParserTest, VaryingComputeAsOutput)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (compute, fragment) {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("cannot use compute stage for varying", messages[0].message);
}

TEST(ParserTest, VaryingComputeAsInput)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (vertex, compute) {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(18U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("cannot use compute stage for varying", messages[0].message);
}

TEST(ParserTest, VaryingWrongOrder)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("varying (fragment, vertex) {}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(1U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("varying output stage 'fragment' not before input stage 'vertex'",
		messages[0].message);
}

TEST(ParserTest, PatchControlPoints)
{
	std::string path = pathStr(exeDir/"test.msl");
	{
		std::stringstream stream("pipeline Test {patch_control_points = 123 ;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(123U, pipelines[0].renderState.patchControlPoints);
	}

	{
		std::stringstream stream("pipeline Test {patch_control_points = 0xAbCd;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_TRUE(parser.parse(output));

		const std::vector<Parser::Pipeline>& pipelines = parser.getPipelines();
		ASSERT_EQ(1U, pipelines.size());
		EXPECT_EQ(0xABCD, pipelines[0].renderState.patchControlPoints);
	}

	{
		std::stringstream stream("pipeline Test {patch_control_points = asdf;}");
		Parser parser;
		Preprocessor preprocessor;
		Output output;
		EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
		EXPECT_FALSE(parser.parse(output));

		const std::vector<Output::Message>& messages = output.getMessages();
		ASSERT_EQ(1U, messages.size());
		EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
		EXPECT_EQ(1U, messages[0].line);
		EXPECT_EQ(39U, messages[0].column);
		EXPECT_EQ("invalid int value: 'asdf'", messages[0].message);
	}
}

TEST(ParserTest, UnnamedSamplerState)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state {min_filter = linear;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(15U, messages[0].column);
	EXPECT_EQ("unexpected token: '{', expected identifier", messages[0].message);
}

TEST(ParserTest, SamplerStateMissingOpenBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state Test min_filter = linear;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(20U, messages[0].column);
	EXPECT_EQ("unexpected token: 'min_filter', expected '{'", messages[0].message);
}

TEST(ParserTest, SamplerStateUnknownState)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state Test {asdf = linear;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(21U, messages[0].column);
	EXPECT_EQ("unknown sampler state name: 'asdf'", messages[0].message);
}

TEST(ParserTest, SamplerStateMissingEquals)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state Test {min_filter linear;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(32U, messages[0].column);
	EXPECT_EQ("unexpected token: 'linear', expected '='", messages[0].message);
}

TEST(ParserTest, SamplerStateMissingValue)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state Test {min_filter =;}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(33U, messages[0].column);
	EXPECT_EQ("unexpected token: ';'", messages[0].message);
}

TEST(ParserTest, SamplerStateMissingSemicolon)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state Test {min_filter = linear}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(40U, messages[0].column);
	EXPECT_EQ("unexpected token: '}'", messages[0].message);
}

TEST(ParserTest, SamplerStateMissingEndBrace)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state Test {min_filter = linear;");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(40U, messages[0].column);
	EXPECT_EQ("unexpected end of file", messages[0].message);
}

TEST(ParserTest, DuplicateSamplerState)
{
	std::string path = pathStr(exeDir/"test.msl");
	std::stringstream stream("sampler_state Test {} sampler_state Test{}");
	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, stream, path));
	EXPECT_FALSE(parser.parse(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(2U, messages.size());
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), path));
	EXPECT_EQ(1U, messages[0].line);
	EXPECT_EQ(37U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("sampler state of name 'Test' already declared", messages[0].message);

	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[1].file), path));
	EXPECT_EQ(1U, messages[1].line);
	EXPECT_EQ(15U, messages[1].column);
	EXPECT_TRUE(messages[1].continued);
	EXPECT_EQ("see other declaration of sampler state 'Test'", messages[1].message);
}

TEST(ParserTest, RemoveUniformBlocks)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output,
		pathStr(inputDir/"RemoveUniformBlocks.msl")));
	EXPECT_TRUE(parser.parse(output, Parser::RemoveUniformBlocks));

	std::vector<Parser::LineMapping> lineMappings;
	Parser::Pipeline pipeline;
	EXPECT_EQ(readFile(outputDir/"RemoveUniformBlocks.vert"),
		parser.createShaderString(lineMappings, output, pipeline, Stage::Vertex, true) + '\n');
}

TEST(ParserTest, LineNumbers)
{
	boost::filesystem::path inputDir = exeDir/"inputs";

	std::string fileName = pathStr(inputDir/"LineNumbers.msl");
	std::string includeFileName = pathStr(inputDir/"LineNumbers.mslh");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(inputDir.string());
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, fileName));
	EXPECT_TRUE(parser.parse(output));

	std::vector<Parser::LineMapping> expectedMappings =
	{
		Parser::LineMapping{"<internal>", 0},
		Parser::LineMapping{"<internal>", 0},
		Parser::LineMapping{includeFileName.c_str(), 13},
		Parser::LineMapping{includeFileName.c_str(), 14},
		Parser::LineMapping{"<internal>", 0},
		Parser::LineMapping{includeFileName.c_str(), 3},
		Parser::LineMapping{includeFileName.c_str(), 4},
		Parser::LineMapping{includeFileName.c_str(), 5},
		Parser::LineMapping{includeFileName.c_str(), 6},
		Parser::LineMapping{includeFileName.c_str(), 7},
		Parser::LineMapping{includeFileName.c_str(), 8},
		Parser::LineMapping{includeFileName.c_str(), 9},
		Parser::LineMapping{includeFileName.c_str(), 10},
		Parser::LineMapping{includeFileName.c_str(), 11},
		Parser::LineMapping{fileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 16},
		Parser::LineMapping{includeFileName.c_str(), 17},
		Parser::LineMapping{includeFileName.c_str(), 18},
		Parser::LineMapping{includeFileName.c_str(), 19},
		Parser::LineMapping{includeFileName.c_str(), 19},
		Parser::LineMapping{includeFileName.c_str(), 20},
		Parser::LineMapping{includeFileName.c_str(), 21},
		Parser::LineMapping{includeFileName.c_str(), 22},
		Parser::LineMapping{includeFileName.c_str(), 22},
		Parser::LineMapping{includeFileName.c_str(), 23},
		Parser::LineMapping{includeFileName.c_str(), 26},
		Parser::LineMapping{fileName.c_str(), 6},
		Parser::LineMapping{fileName.c_str(), 7},
		Parser::LineMapping{fileName.c_str(), 8},
		Parser::LineMapping{fileName.c_str(), 9},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 18},
		Parser::LineMapping{fileName.c_str(), 19},
		Parser::LineMapping{fileName.c_str(), 20},
		Parser::LineMapping{fileName.c_str(), 23},
		Parser::LineMapping{fileName.c_str(), 26}
	};

	std::vector<Parser::LineMapping> lineMappings;
	Parser::Pipeline pipeline;
	parser.createShaderString(lineMappings, output, pipeline, Stage::Vertex, true);

	ASSERT_EQ(expectedMappings.size(), lineMappings.size());
	for (std::size_t i = 0; i < expectedMappings.size(); ++i)
	{
		EXPECT_TRUE(boost::algorithm::ends_with(lineMappings[i].fileName,
			expectedMappings[i].fileName));
		EXPECT_EQ(expectedMappings[i].line, lineMappings[i].line);
	}
}

TEST(ParserTest, LineNumbersRemoveUniformBlocks)
{
	boost::filesystem::path inputDir = exeDir/"inputs";

	std::string fileName = pathStr(inputDir/"LineNumbers.msl");
	std::string includeFileName = pathStr(inputDir/"LineNumbers.mslh");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	preprocessor.addIncludePath(inputDir.string());
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, fileName));
	EXPECT_TRUE(parser.parse(output, Parser::RemoveUniformBlocks));

	std::vector<Parser::LineMapping> expectedMappings =
	{
		Parser::LineMapping{"<internal>", 0},
		Parser::LineMapping{"<internal>", 0},
		Parser::LineMapping{includeFileName.c_str(), 13},
		Parser::LineMapping{includeFileName.c_str(), 14},
		Parser::LineMapping{includeFileName.c_str(), 7},
		Parser::LineMapping{includeFileName.c_str(), 8},
		Parser::LineMapping{"<internal>", 0},
		Parser::LineMapping{fileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 1},
		Parser::LineMapping{includeFileName.c_str(), 16},
		Parser::LineMapping{includeFileName.c_str(), 17},
		Parser::LineMapping{includeFileName.c_str(), 18},
		Parser::LineMapping{includeFileName.c_str(), 19},
		Parser::LineMapping{includeFileName.c_str(), 19},
		Parser::LineMapping{includeFileName.c_str(), 20},
		Parser::LineMapping{includeFileName.c_str(), 21},
		Parser::LineMapping{includeFileName.c_str(), 22},
		Parser::LineMapping{includeFileName.c_str(), 22},
		Parser::LineMapping{includeFileName.c_str(), 23},
		Parser::LineMapping{includeFileName.c_str(), 26},
		Parser::LineMapping{fileName.c_str(), 6},
		Parser::LineMapping{fileName.c_str(), 7},
		Parser::LineMapping{fileName.c_str(), 8},
		Parser::LineMapping{fileName.c_str(), 9},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 16},
		Parser::LineMapping{fileName.c_str(), 18},
		Parser::LineMapping{fileName.c_str(), 19},
		Parser::LineMapping{fileName.c_str(), 20},
		Parser::LineMapping{fileName.c_str(), 23},
		Parser::LineMapping{fileName.c_str(), 26}
	};

	std::vector<Parser::LineMapping> lineMappings;
	Parser::Pipeline pipeline;
	parser.createShaderString(lineMappings, output, pipeline, Stage::Vertex, true);

	ASSERT_EQ(expectedMappings.size(), lineMappings.size());
	for (std::size_t i = 0; i < expectedMappings.size(); ++i)
	{
		EXPECT_TRUE(boost::algorithm::ends_with(lineMappings[i].fileName,
			expectedMappings[i].fileName));
		EXPECT_EQ(expectedMappings[i].line, lineMappings[i].line);
	}
}

} // namespace msl
