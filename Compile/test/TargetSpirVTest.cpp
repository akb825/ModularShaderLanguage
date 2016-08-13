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
#include <MSL/Compile/TargetSpirV.h>
#include <gtest/gtest.h>
#include <sstream>

namespace msl
{

TEST(TargetSpirVTest, CompleteShader)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompleteShader.msl").string();

	TargetSpirV target;
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

TEST(TargetSpirVTest, VersionNumber)
{
	std::stringstream stream(
		"#if SPIRV_VERSION == 1\n"
		"#error Version correctly set.\n"
		"#endif");
	TargetSpirV target;

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, "test.msl"));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ((exeDir/"test.msl").string(), messages[0].file);
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Version correctly set.", messages[0].message);
}

TEST(TargetSpirVTest, CompileError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompileError.msl").string();

	TargetSpirV target;
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ((inputDir/"CompileError.mslh").string(), messages[0].file);
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'inputss' : undeclared identifier", messages[0].message);
}

TEST(TargetSpirVTest, CompileWarning)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompileWarning.msl").string();

	TargetSpirV target;
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Warning, messages[0].level);
	EXPECT_EQ((inputDir/"CompileWarning.mslh").string(), messages[0].file);
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'switch' : last case/default label not followed by statements", messages[0].message);
}

TEST(TargetSpirVTest, LinkError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"LinkError.msl").string();

	TargetSpirV target;
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ((inputDir/"LinkError.mslh").string(), messages[0].file);
	EXPECT_EQ(5U, messages[0].line);
	EXPECT_EQ("Linking fragment stage: Missing entry point", messages[0].message);
}

TEST(TargetSpirVTest, ResourcesNotFound)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompleteShader.msl").string();

	TargetSpirV target;
	target.addIncludePath(inputDir.string());
	target.setResourcesFileName("asdf");

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ("cannot read resources file: asdf", messages[0].message);
}

TEST(TargetSpirVTest, Resources)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompleteShader.msl").string();

	TargetSpirV target;
	target.addIncludePath(inputDir.string());
	target.setResourcesFileName((inputDir/"Resources.conf").string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
}

TEST(TargetSpirVTest, InvalidResources)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompleteShader.msl").string();

	TargetSpirV target;
	target.addIncludePath(inputDir.string());
	target.setResourcesFileName((inputDir/"InvalidResources.conf").string());

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ((inputDir/"InvalidResources.conf").string(), messages[0].file);
	EXPECT_EQ(4U, messages[0].line);
	EXPECT_EQ("resource configuration syntax error: each name must be followed by one number",
		messages[0].message);
}

TEST(TargetSpirVTest, DuplicatePipeline)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = (inputDir/"CompleteShader.msl").string();

	TargetSpirV target;
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(2U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(shaderName, messages[0].file);
	EXPECT_EQ(37U, messages[0].line);
	EXPECT_EQ("pipeline already declared: Test", messages[0].message);

	EXPECT_EQ(Output::Level::Error, messages[1].level);
	EXPECT_TRUE(messages[1].continued);
	EXPECT_EQ(shaderName, messages[1].file);
	EXPECT_EQ(37U, messages[1].line);
	EXPECT_EQ("see previous declaration", messages[1].message);
}

} // namespace msl
