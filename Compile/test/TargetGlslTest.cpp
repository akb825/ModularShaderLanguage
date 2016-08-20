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
#include <MSL/Compile/CompiledResult.h>
#include <MSL/Compile/Output.h>
#include <MSL/Compile/TargetGlsl.h>
#include <gtest/gtest.h>

namespace msl
{

TEST(TargetGlslTest, Glsl450)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetGlsl target(450, false);
	target.addIncludePath(inputDir.string());
	target.addHeaderLine("precision mediump float;");
	target.addHeaderLine(Target::Stage::Fragment, "precision mediump sampler2D;");

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto noShader = CompiledResult::noShader;
	auto unknown = CompiledResult::unknown;
	EXPECT_EQ(1U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.vertex);
	EXPECT_EQ(noShader, pipeline->second.tessellationControl);
	EXPECT_EQ(noShader, pipeline->second.tessellationEvaluation);
	EXPECT_EQ(noShader, pipeline->second.geometry);
	EXPECT_EQ(1U, pipeline->second.fragment);
	EXPECT_EQ(noShader, pipeline->second.compute);

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Transform.transform", pipeline->second.uniforms[0].name);
	EXPECT_EQ(CompiledResult::Type::Mat4, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].blockIndex);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[0].elements);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(CompiledResult::Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].blockIndex);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[1].elements);

	ASSERT_EQ(1U, pipeline->second.uniformBlocks.size());
	EXPECT_EQ("Transform", pipeline->second.uniformBlocks[0].name);
	EXPECT_EQ(sizeof(float)*16, pipeline->second.uniformBlocks[0].size);

	ASSERT_EQ(2U, pipeline->second.attributes.size());
	EXPECT_EQ("position", pipeline->second.attributes[0].name);
	EXPECT_EQ(CompiledResult::Type::Vec3, pipeline->second.attributes[0].type);

	EXPECT_EQ("color", pipeline->second.attributes[1].name);
	EXPECT_EQ(CompiledResult::Type::Vec4, pipeline->second.attributes[1].type);

	ASSERT_EQ(2U, result.getShaders().size());
	std::string vertex = reinterpret_cast<const char*>(result.getShaders()[0].data());
	std::string fragment = reinterpret_cast<const char*>(result.getShaders()[1].data());
	EXPECT_NE(std::string::npos, vertex.find("precision mediump float;"));
	EXPECT_EQ(std::string::npos, vertex.find("precision mediump sampler2D;"));
	EXPECT_NE(std::string::npos, fragment.find("precision mediump float;"));
	EXPECT_NE(std::string::npos, fragment.find("precision mediump sampler2D;"));
}

TEST(TargetGlslTest, Glsl450VersionNumber)
{
	std::stringstream stream(
		"#if GLSL_VERSION == 450\n"
		"#error Version correctly set.\n"
		"#endif");
	TargetGlsl target(450, false);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(exeDir/"test.msl"), messages[0].file);
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Version correctly set.", messages[0].message);
}

TEST(TargetGlslTest, Glsl120)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetGlsl target(120, false);
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto noShader = CompiledResult::noShader;
	auto unknown = CompiledResult::unknown;
	EXPECT_EQ(1U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.vertex);
	EXPECT_EQ(noShader, pipeline->second.tessellationControl);
	EXPECT_EQ(noShader, pipeline->second.tessellationEvaluation);
	EXPECT_EQ(noShader, pipeline->second.geometry);
	EXPECT_EQ(1U, pipeline->second.fragment);
	EXPECT_EQ(noShader, pipeline->second.compute);

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Transform.transform", pipeline->second.uniforms[0].name);
	EXPECT_EQ(CompiledResult::Type::Mat4, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].blockIndex);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[0].elements);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(CompiledResult::Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].blockIndex);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[1].elements);

	ASSERT_EQ(1U, pipeline->second.uniformBlocks.size());
	EXPECT_EQ("Transform", pipeline->second.uniformBlocks[0].name);
	EXPECT_EQ(sizeof(float)*16, pipeline->second.uniformBlocks[0].size);

	ASSERT_EQ(2U, pipeline->second.attributes.size());
	EXPECT_EQ("position", pipeline->second.attributes[0].name);
	EXPECT_EQ(CompiledResult::Type::Vec3, pipeline->second.attributes[0].type);

	EXPECT_EQ("color", pipeline->second.attributes[1].name);
	EXPECT_EQ(CompiledResult::Type::Vec4, pipeline->second.attributes[1].type);

	EXPECT_EQ(2U, result.getShaders().size());
}

TEST(TargetGlslTest, Glsl120VersionNumber)
{
	std::stringstream stream(
		"#if GLSL_VERSION == 120\n"
		"#error Version correctly set.\n"
		"#endif");
	TargetGlsl target(120, false);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(exeDir/"test.msl"), messages[0].file);
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Version correctly set.", messages[0].message);
}

TEST(TargetGlslTest, GlslEs300)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetGlsl target(300, true);
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto noShader = CompiledResult::noShader;
	auto unknown = CompiledResult::unknown;
	EXPECT_EQ(1U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.vertex);
	EXPECT_EQ(noShader, pipeline->second.tessellationControl);
	EXPECT_EQ(noShader, pipeline->second.tessellationEvaluation);
	EXPECT_EQ(noShader, pipeline->second.geometry);
	EXPECT_EQ(1U, pipeline->second.fragment);
	EXPECT_EQ(noShader, pipeline->second.compute);

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Transform.transform", pipeline->second.uniforms[0].name);
	EXPECT_EQ(CompiledResult::Type::Mat4, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].blockIndex);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[0].elements);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(CompiledResult::Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].blockIndex);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[1].elements);

	ASSERT_EQ(1U, pipeline->second.uniformBlocks.size());
	EXPECT_EQ("Transform", pipeline->second.uniformBlocks[0].name);
	EXPECT_EQ(sizeof(float)*16, pipeline->second.uniformBlocks[0].size);

	ASSERT_EQ(2U, pipeline->second.attributes.size());
	EXPECT_EQ("position", pipeline->second.attributes[0].name);
	EXPECT_EQ(CompiledResult::Type::Vec3, pipeline->second.attributes[0].type);

	EXPECT_EQ("color", pipeline->second.attributes[1].name);
	EXPECT_EQ(CompiledResult::Type::Vec4, pipeline->second.attributes[1].type);

	EXPECT_EQ(2U, result.getShaders().size());
}

TEST(TargetGlslTest, GlslEs300VersionNumber)
{
	std::stringstream stream(
		"#if GLSLES_VERSION == 300\n"
		"#error Version correctly set.\n"
		"#endif");
	TargetGlsl target(300, true);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(exeDir/"test.msl"), messages[0].file);
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Version correctly set.", messages[0].message);
}

TEST(TargetGlslTest, GlslEs100)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetGlsl target(100, true);
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto noShader = CompiledResult::noShader;
	auto unknown = CompiledResult::unknown;
	EXPECT_EQ(1U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.vertex);
	EXPECT_EQ(noShader, pipeline->second.tessellationControl);
	EXPECT_EQ(noShader, pipeline->second.tessellationEvaluation);
	EXPECT_EQ(noShader, pipeline->second.geometry);
	EXPECT_EQ(1U, pipeline->second.fragment);
	EXPECT_EQ(noShader, pipeline->second.compute);

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Transform.transform", pipeline->second.uniforms[0].name);
	EXPECT_EQ(CompiledResult::Type::Mat4, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].blockIndex);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[0].elements);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(CompiledResult::Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].blockIndex);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].bufferOffset);
	EXPECT_EQ(1U, pipeline->second.uniforms[1].elements);

	ASSERT_EQ(1U, pipeline->second.uniformBlocks.size());
	EXPECT_EQ("Transform", pipeline->second.uniformBlocks[0].name);
	EXPECT_EQ(sizeof(float)*16, pipeline->second.uniformBlocks[0].size);

	ASSERT_EQ(2U, pipeline->second.attributes.size());
	EXPECT_EQ("position", pipeline->second.attributes[0].name);
	EXPECT_EQ(CompiledResult::Type::Vec3, pipeline->second.attributes[0].type);

	EXPECT_EQ("color", pipeline->second.attributes[1].name);
	EXPECT_EQ(CompiledResult::Type::Vec4, pipeline->second.attributes[1].type);

	EXPECT_EQ(2U, result.getShaders().size());
}

TEST(TargetGlslTest, GlslEs100VersionNumber)
{
	std::stringstream stream(
		"#if GLSLES_VERSION == 100\n"
		"#error Version correctly set.\n"
		"#endif");
	TargetGlsl target(100, true);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(exeDir/"test.msl"), messages[0].file);
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Version correctly set.", messages[0].message);
}

TEST(TargetGlslTest, Glsl450HasUniformBuffers)
{
	std::stringstream stream(
		"#if HAS_UNIFORM_BUFFERS\n"
		"#error Has buffers set.\n"
		"#else\n"
		"#error Has buffers not set.\n"
		"#endif");
	TargetGlsl target(450, false);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(exeDir/"test.msl"), messages[0].file);
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Has buffers set.", messages[0].message);
}

TEST(TargetGlslTest, Glsl120HasUniformBuffers)
{
	std::stringstream stream(
		"#if HAS_UNIFORM_BUFFERS\n"
		"#error Has buffers set.\n"
		"#else\n"
		"#error Has buffers not set.\n"
		"#endif");
	TargetGlsl target(120, false);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(exeDir/"test.msl"), messages[0].file);
	EXPECT_EQ(4U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Has buffers not set.", messages[0].message);
}

TEST(TargetGlslTest, CompileError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompileError.msl");

	TargetGlsl target(450, false);
	target.addIncludePath(pathStr(inputDir));

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(inputDir/"CompileError.mslh"), pathStr(messages[0].file));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'inputss' : undeclared identifier", messages[0].message);
}

TEST(TargetGlslTest, CompileWarning)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompileWarning.msl");

	TargetGlsl target(450, false);
	target.addIncludePath(pathStr(inputDir));

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Warning, messages[0].level);
	EXPECT_EQ(pathStr(inputDir/"CompileWarning.mslh"), pathStr(messages[0].file));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'switch' : last case/default label not followed by statements", messages[0].message);
}

TEST(TargetGlslTest, LinkError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkError.msl");

	TargetGlsl target(450, false);
	target.addIncludePath(pathStr(inputDir));

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(pathStr(inputDir/"LinkError.mslh"), pathStr(messages[0].file));
	EXPECT_EQ(5U, messages[0].line);
	EXPECT_EQ("Linking fragment stage: Missing entry point", messages[0].message);
}

} // namespace msl
