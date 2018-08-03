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
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>

namespace msl
{

using namespace compile;

TEST(TargetGlslTest, Glsl450)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"CompleteShader.msl");

	TargetGlsl target(450, false);
	target.addIncludePath(inputDir.string());
	target.addHeaderLine("precision mediump float;");
	target.addHeaderLine(Stage::Fragment, "precision mediump sampler2D;");

	Output output;
	CompiledResult result;
	EXPECT_TRUE(target.compile(result, output, shaderName));
	shaderName = pathStr(inputDir/"SecondCompleteShader.msl");
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto unknown = msl::compile::unknown;
	EXPECT_EQ(2U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.shaders[static_cast<int>(Stage::Vertex)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(1U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Transform", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
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
	EXPECT_EQ(0, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(2U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Transform", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
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
	EXPECT_EQ(0, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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

	ASSERT_EQ(3U, result.getShaders().size());
	std::string vertex = reinterpret_cast<const char*>(result.getShaders()[0].data());
	std::string fragment = reinterpret_cast<const char*>(result.getShaders()[1].data());
	std::string fragment2 = reinterpret_cast<const char*>(result.getShaders()[2].data());
	EXPECT_NE(std::string::npos, vertex.find("precision mediump float;"));
	EXPECT_EQ(std::string::npos, vertex.find("precision mediump sampler2D;"));
	EXPECT_NE(std::string::npos, fragment.find("precision mediump float;"));
	EXPECT_NE(std::string::npos, fragment.find("precision mediump sampler2D;"));
	EXPECT_NE(std::string::npos, fragment2.find("precision mediump float;"));
	EXPECT_NE(std::string::npos, fragment2.find("precision mediump sampler2D;"));
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
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), pathStr(exeDir/"test.msl")));
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
	shaderName = pathStr(inputDir/"SecondCompleteShader.msl");
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto unknown = msl::compile::unknown;
	EXPECT_EQ(2U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.shaders[static_cast<int>(Stage::Vertex)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(1U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Uniforms", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].members[0].size);
	EXPECT_EQ(Type::Mat4, pipeline->second.structs[0].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[0].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[0].arrayElements.empty());

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Uniforms", pipeline->second.uniforms[0].name);
	EXPECT_EQ(UniformType::PushConstant, pipeline->second.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[0].arrayElements.empty());
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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

	EXPECT_EQ(0U, pipeline->second.pushConstantStruct);

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
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(2U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Uniforms", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].members[0].size);
	EXPECT_EQ(Type::Mat4, pipeline->second.structs[0].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[0].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[0].arrayElements.empty());

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Uniforms", pipeline->second.uniforms[0].name);
	EXPECT_EQ(UniformType::PushConstant, pipeline->second.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[0].arrayElements.empty());
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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

	EXPECT_EQ(0U, pipeline->second.pushConstantStruct);

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
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), pathStr(exeDir/"test.msl")));
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
	shaderName = pathStr(inputDir/"SecondCompleteShader.msl");
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto unknown = msl::compile::unknown;
	EXPECT_EQ(2U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.shaders[static_cast<int>(Stage::Vertex)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(1U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Transform", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
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
	EXPECT_EQ(0, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(2U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Transform", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
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
	EXPECT_EQ(0, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), pathStr(exeDir/"test.msl")));
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
	shaderName = pathStr(inputDir/"SecondCompleteShader.msl");
	EXPECT_TRUE(target.compile(result, output, shaderName));
	EXPECT_TRUE(target.finish(result, output));

	EXPECT_EQ(0, output.getMessages().size());

	auto unknown = msl::compile::unknown;
	EXPECT_EQ(2U, result.getPipelines().size());
	auto pipeline = result.getPipelines().find("Test");
	ASSERT_NE(pipeline, result.getPipelines().end());
	EXPECT_EQ(0U, pipeline->second.shaders[static_cast<int>(Stage::Vertex)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(1U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Uniforms", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].members[0].size);
	EXPECT_EQ(Type::Mat4, pipeline->second.structs[0].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[0].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[0].arrayElements.empty());

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Uniforms", pipeline->second.uniforms[0].name);
	EXPECT_EQ(UniformType::PushConstant, pipeline->second.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[0].arrayElements.empty());
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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

	EXPECT_EQ(0U, pipeline->second.pushConstantStruct);

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
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationControl)].shader);
	EXPECT_EQ(unknown,
		pipeline->second.shaders[static_cast<int>(Stage::TessellationEvaluation)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Geometry)].shader);
	EXPECT_EQ(2U, pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader);
	EXPECT_EQ(unknown, pipeline->second.shaders[static_cast<int>(Stage::Compute)].shader);

	ASSERT_EQ(1U, pipeline->second.structs.size());
	EXPECT_EQ("Uniforms", pipeline->second.structs[0].name);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].size);

	ASSERT_EQ(1U, pipeline->second.structs[0].members.size());
	EXPECT_EQ("transform", pipeline->second.structs[0].members[0].name);
	EXPECT_EQ(0, pipeline->second.structs[0].members[0].offset);
	EXPECT_EQ(16*sizeof(float), pipeline->second.structs[0].members[0].size);
	EXPECT_EQ(Type::Mat4, pipeline->second.structs[0].members[0].type);
	EXPECT_EQ(unknown, pipeline->second.structs[0].members[0].structIndex);
	EXPECT_TRUE(pipeline->second.structs[0].members[0].arrayElements.empty());

	ASSERT_EQ(2U, pipeline->second.uniforms.size());
	EXPECT_EQ("Uniforms", pipeline->second.uniforms[0].name);
	EXPECT_EQ(UniformType::PushConstant, pipeline->second.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, pipeline->second.uniforms[0].type);
	EXPECT_EQ(0U, pipeline->second.uniforms[0].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[0].arrayElements.empty());
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].descriptorSet);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].binding);
	EXPECT_EQ(unknown, pipeline->second.uniforms[0].samplerIndex);

	EXPECT_EQ("tex", pipeline->second.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, pipeline->second.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, pipeline->second.uniforms[1].type);
	EXPECT_EQ(unknown, pipeline->second.uniforms[1].structIndex);
	EXPECT_TRUE(pipeline->second.uniforms[1].arrayElements.empty());
	EXPECT_EQ(0, pipeline->second.uniforms[1].descriptorSet);
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

	EXPECT_EQ(0U, pipeline->second.pushConstantStruct);

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
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), pathStr(exeDir/"test.msl")));
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Version correctly set.", messages[0].message);
}

TEST(TargetGlslTest, Glsl450HasUniformBlocks)
{
	std::stringstream stream(
		"#if HAS_UNIFORM_BLOCKS\n"
		"#error Has blocks set.\n"
		"#else\n"
		"#error Has blocks not set.\n"
		"#endif");
	TargetGlsl target(450, false);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(messages[0].file, pathStr(exeDir/"test.msl")));
	EXPECT_EQ(2U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Has blocks set.", messages[0].message);
}

TEST(TargetGlslTest, Glsl120HasUniformBlocks)
{
	std::stringstream stream(
		"#if HAS_UNIFORM_BLOCKS\n"
		"#error Has blocks set.\n"
		"#else\n"
		"#error Has blocks not set.\n"
		"#endif");
	TargetGlsl target(120, false);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, stream, pathStr(exeDir/"test.msl")));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_LE(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), pathStr(exeDir/"test.msl")));
	EXPECT_EQ(4U, messages[0].line);
	EXPECT_EQ("encountered #error directive: Has blocks not set.", messages[0].message);
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
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"CompileError.mslh")));
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
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file),
		pathStr(inputDir/"CompileWarning.mslh")));
	EXPECT_EQ(15U, messages[0].line);
	EXPECT_EQ("'switch' : last case/default label not followed by statements", messages[0].message);
}

TEST(TargetGlslTest, MissingEntryPoint)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"MissingEntryPoint.msl");

	TargetGlsl target(450, false);
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

TEST(TargetGlslTest, DuplicateEntryPoint)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"DuplicateEntryPoint.msl");

	TargetGlsl target(450, false);
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

} // namespace msl
