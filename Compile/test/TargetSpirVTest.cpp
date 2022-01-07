/*
 * Copyright 2016-2022 Aaron Barany
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
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>
#include <sstream>

namespace msl
{

using namespace compile;

TEST(TargetSpirVTest, CompleteShader)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetSpirV target;
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	shaderName = pathStr(inputDir/"SecondCompleteShader.msl");
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0U, output.getMessages().size());

	auto unknown = msl::compile::unknown;
	EXPECT_EQ(2U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.shaders[static_cast<int>(Stage::Vertex)].shader);
	EXPECT_EQ(noShader,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(noShader,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(noShader, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(1U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(noShader, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Transform", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0U, pipeline->second.structs[0].members[0].offset);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].members[0].size);
	EXPECT_EQ(Type::Mat4, pipeline->second.structs[0].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[0].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[0].arrayElements.empty());

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Transform", pipeline->second.uniforms[0].name);
	EXPECT_EQ(UniformType::Block, pipeline->second.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[0].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.uniforms[1].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].binding);
	EXPECT_EQ(0U, pipeline->second.uniforms[1].samplerIndex);

	ASSERT_EQ(2U, pipeline->second.attributes.size());
	EXPECT_EQ("position", pipeline->second.attributes[0].name);
	EXPECT_EQ(Type::Vec3, pipeline->second.attributes[0].type);
	EXPECT_TRUE(pipeline->second.attributes[0].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.attributes[0].location);
	EXPECT_EQ(0U, pipeline->second.attributes[0].component);

	EXPECT_EQ("color", pipeline->second.attributes[1].name);
	EXPECT_EQ(Type::Vec4, pipeline->second.attributes[1].type);
	EXPECT_TRUE(pipeline->second.attributes[1].arrayElements.empty());
	EXPECT_EQ(1U, pipeline->second.attributes[1].location);
	EXPECT_EQ(0U, pipeline->second.attributes[1].component);

	ASSERT_EQ(1U, pipeline->second.fragmentOutputs.size());
	EXPECT_EQ("color", pipeline->second.fragmentOutputs[0].name);
	EXPECT_EQ(0U, pipeline->second.fragmentOutputs[0].location);

	EXPECT_EQ(unknown, pipeline->second.pushConstantStruct);

	ASSERT_EQ(1U, pipeline->second.samplerStates.size());
	EXPECT_EQ(Filter::Linear, pipeline->second.samplerStates[0].minFilter);
	EXPECT_EQ(Filter::Linear, pipeline->second.samplerStates[0].magFilter);
	EXPECT_EQ(MipFilter::Anisotropic, pipeline->second.samplerStates[0].mipFilter);
	EXPECT_EQ(AddressMode::Repeat, pipeline->second.samplerStates[0].addressModeU);
	EXPECT_EQ(AddressMode::ClampToEdge, pipeline->second.samplerStates[0].addressModeV);
	EXPECT_EQ(AddressMode::Unset, pipeline->second.samplerStates[0].addressModeW);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].mipLodBias);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].maxAnisotropy);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].minLod);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].maxLod);
	EXPECT_EQ(BorderColor::Unset, pipeline->second.samplerStates[0].borderColor);
	EXPECT_EQ(CompareOp::Unset, pipeline->second.samplerStates[0].compareOp);

	pipeline = result.getPipelines().find("Test2");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.shaders[static_cast<int>(Stage::Vertex)].shader);
	EXPECT_EQ(noShader,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(noShader,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(noShader, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(2U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(noShader, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Transform", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0U, pipeline->second.structs[0].members[0].offset);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].members[0].size);
	EXPECT_EQ(Type::Mat4, pipeline->second.structs[0].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[0].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[0].arrayElements.empty());

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Transform", pipeline->second.uniforms[0].name);
	EXPECT_EQ(UniformType::Block, pipeline->second.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[0].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.uniforms[1].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].binding);
	EXPECT_EQ(0U, pipeline->second.uniforms[1].samplerIndex);

	ASSERT_EQ(2U, pipeline->second.attributes.size());
	EXPECT_EQ("position", pipeline->second.attributes[0].name);
	EXPECT_EQ(Type::Vec3, pipeline->second.attributes[0].type);
	EXPECT_TRUE(pipeline->second.attributes[0].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.attributes[0].location);
	EXPECT_EQ(0U, pipeline->second.attributes[0].component);

	EXPECT_EQ("color", pipeline->second.attributes[1].name);
	EXPECT_EQ(Type::Vec4, pipeline->second.attributes[1].type);
	EXPECT_TRUE(pipeline->second.attributes[1].arrayElements.empty());
	EXPECT_EQ(1U, pipeline->second.attributes[1].location);
	EXPECT_EQ(0U, pipeline->second.attributes[1].component);

	ASSERT_EQ(1U, pipeline->second.fragmentOutputs.size());
	EXPECT_EQ("color", pipeline->second.fragmentOutputs[0].name);
	EXPECT_EQ(0U, pipeline->second.fragmentOutputs[0].location);

	EXPECT_EQ(unknown, pipeline->second.pushConstantStruct);

	ASSERT_EQ(1U, pipeline->second.samplerStates.size());
	EXPECT_EQ(Filter::Nearest, pipeline->second.samplerStates[0].minFilter);
	EXPECT_EQ(Filter::Nearest, pipeline->second.samplerStates[0].magFilter);
	EXPECT_EQ(MipFilter::None, pipeline->second.samplerStates[0].mipFilter);
	EXPECT_EQ(AddressMode::ClampToEdge, pipeline->second.samplerStates[0].addressModeU);
	EXPECT_EQ(AddressMode::Repeat, pipeline->second.samplerStates[0].addressModeV);
	EXPECT_EQ(AddressMode::Unset, pipeline->second.samplerStates[0].addressModeW);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].mipLodBias);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].maxAnisotropy);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].minLod);
	EXPECT_EQ(unknownFloat, pipeline->second.samplerStates[0].maxLod);
	EXPECT_EQ(BorderColor::Unset, pipeline->second.samplerStates[0].borderColor);
	EXPECT_EQ(CompareOp::Unset, pipeline->second.samplerStates[0].compareOp);

	EXPECT_EQ(3U, result.getShaders().size());
}

TEST(TargetSpirVTest, CombineReflection)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CombineReflection.msl");

	TargetSpirV target;
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0U, output.getMessages().size());

	auto unknown = msl::compile::unknown;
	EXPECT_EQ(1U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.shaders[static_cast<int>(Stage::Vertex)].shader);
	EXPECT_EQ(noShader,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(noShader,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(noShader, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(1U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(noShader, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(5U, pipeline->second.structs.size());
	EXPECT_EQ("VertexUniform", pipeline->second.structs[0].name);
	EXPECT_EQ(8*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(2U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("member", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0U, pipeline->second.structs[0].members[0].offset);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[0].members[0].size);
	EXPECT_EQ(Type::Struct, pipeline->second.structs[0].members[0].type);
	EXPECT_EQ(1U, pipeline->second.structs[0].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[0].arrayElements.empty());

	EXPECT_EQ("otherMember", pipeline->second.structs[0].members[1].name);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[0].members[1].offset);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[0].members[1].size);
	EXPECT_EQ(Type::Struct, pipeline->second.structs[0].members[1].type);
	EXPECT_EQ(2U, pipeline->second.structs[0].members[1].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[1].arrayElements.empty());

	EXPECT_EQ("TestStruct", pipeline->second.structs[1].name);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[1].size);

	ASSERT_EQ(1U, pipeline->second.structs[1].members.size());
	EXPECT_EQ("value", pipeline->second.structs[1].members[0].name);
	EXPECT_EQ(0U, pipeline->second.structs[1].members[0].offset);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[1].members[0].size);
	EXPECT_EQ(Type::Vec4, pipeline->second.structs[1].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[1].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[1].members[0].arrayElements.empty());

	EXPECT_EQ("VertexOnlyStruct", pipeline->second.structs[2].name);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[2].size);

	ASSERT_EQ(1U, pipeline->second.structs[2].members.size());
	EXPECT_EQ("value", pipeline->second.structs[2].members[0].name);
	EXPECT_EQ(0U, pipeline->second.structs[2].members[0].offset);
	EXPECT_EQ(3*sizeof(float), pipeline->second.structs[2].members[0].size);
	EXPECT_EQ(Type::Vec3, pipeline->second.structs[2].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[2].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[2].members[0].arrayElements.empty());

	EXPECT_EQ("FragmentUniform", pipeline->second.structs[3].name);
	EXPECT_EQ(8*sizeof(float), pipeline->second.structs[3].size);

	ASSERT_EQ(2U, pipeline->second.structs[3].members.size());
	EXPECT_EQ("member", pipeline->second.structs[3].members[0].name);
	EXPECT_EQ(0U, pipeline->second.structs[3].members[0].offset);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[3].members[0].size);
	EXPECT_EQ(Type::Struct, pipeline->second.structs[3].members[0].type);
	EXPECT_EQ(1U, pipeline->second.structs[3].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[3].members[0].arrayElements.empty());

	EXPECT_EQ("otherMember", pipeline->second.structs[3].members[1].name);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[3].members[1].offset);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[3].members[1].size);
	EXPECT_EQ(Type::Struct, pipeline->second.structs[3].members[1].type);
	EXPECT_EQ(4U, pipeline->second.structs[3].members[1].structIndex);
	EXPECT_TRUE(pipeline->second.structs[3].members[1].arrayElements.empty());

	EXPECT_EQ("FragmentOnlyStruct", pipeline->second.structs[4].name);
	EXPECT_EQ(4*sizeof(float), pipeline->second.structs[4].size);

	ASSERT_EQ(1U, pipeline->second.structs[4].members.size());
	EXPECT_EQ("value", pipeline->second.structs[4].members[0].name);
	EXPECT_EQ(0U, pipeline->second.structs[4].members[0].offset);
	EXPECT_EQ(2*sizeof(float), pipeline->second.structs[4].members[0].size);
	EXPECT_EQ(Type::Vec2, pipeline->second.structs[4].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[4].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[4].members[0].arrayElements.empty());

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("VertexUniform", pipeline->second.uniforms[0].name);
	EXPECT_EQ(UniformType::Block, pipeline->second.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[0].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("FragmentUniform", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::Block, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[1].type);
	EXPECT_EQ(3U, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0U, pipeline->second.uniforms[1].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].samplerIndex);

	EXPECT_EQ(2U, result.getShaders().size());
}

TEST(TargetSpirVTest, VersionNumber)
{
	std::stringstream stream(
		"#if SPIRV_VERSION >= 100\n"
		"#error Version correctly set.\n"
		"#endif");
	TargetSpirV target;

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), pathStr(exeDir/"test.msl")));
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Version correctly set.", messages[0].message);
}

TEST(TargetSpirVTest, CompileError)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompileError.msl");

	TargetSpirV target;
	target.addIncludePath(pathStr(inputDir));

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"CompileError.mslh")));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'inputss' : undeclared identifier", messages[0].message);
}

TEST(TargetSpirVTest, CompileWarning)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompileWarning.msl");

	TargetSpirV target;
	target.addIncludePath(pathStr(inputDir));

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Warning, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"CompileWarning.mslh")));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'switch' : last case/default label not followed by statements", messages[0].message);
}

TEST(TargetSpirVTest, MissingEntryPoint)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"MissingEntryPoint.msl");

	TargetSpirV target;
	target.addIncludePath(pathStr(inputDir));

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"MissingEntryPoint.mslh")));
	EXPECT_EQ(8U, messages[0].line);
	EXPECT_EQ("entry point 'fragShader' not found", messages[0].message);
}

TEST(TargetSpirVTest, DuplicateEntryPoint)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"DuplicateEntryPoint.msl");

	TargetSpirV target;
	target.addIncludePath(pathStr(inputDir));

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"DuplicateEntryPoint.mslh")));
	EXPECT_EQ(8U, messages[0].line);
	EXPECT_EQ("entry point 'fragShader' found multiple times", messages[0].message);
}

TEST(TargetSpirVTest, PushConstantMismatch)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"PushConstantMismatch.msl");

	TargetSpirV target;

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(18U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_EQ("linker error: struct Uniforms has different declarations between stages",
		messages[0].message);
}

TEST(TargetSpirVTest, ResourcesNotFound)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetSpirV target;
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
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetSpirV target;
	target.setResourcesFileName(pathStr(inputDir/"Resources.conf"));

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
}

TEST(TargetSpirVTest, InvalidResources)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetSpirV target;
	target.addIncludePath(inputDir.string());
	target.setResourcesFileName(pathStr(inputDir/"InvalidResources.conf"));

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"InvalidResources.conf")));
	EXPECT_EQ(4U, messages[0].line);
	EXPECT_EQ("resource configuration syntax error: each name must be followed by one number",
		messages[0].message);
}

TEST(TargetSpirVTest, DuplicatePipeline)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetSpirV target;
	target.addIncludePath(inputDir.string());

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_FALSE(target.compile(result, output, shaderName));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(2U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(46U, messages[0].line);
	EXPECT_EQ("pipeline already declared: Test", messages[0].message);

	EXPECT_EQ(Output::Level::Error, messages[1].level);
	EXPECT_TRUE(messages[1].continued);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[1].file), shaderName));
	EXPECT_EQ(46U, messages[1].line);
	EXPECT_EQ("see previous declaration", messages[1].message);
}

} // namespace msl
