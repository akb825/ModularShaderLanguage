/*
 * Copyright 2022 Aaron Barany
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
#include <MSL/Compile/TargetMetal.h>
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>

namespace msl
{

namespace
{

class MockTargetMetal : public TargetMetal
{
public:
	MockTargetMetal(std::uint32_t version, Platform platform)
		: TargetMetal(version, platform)
	{
	}

protected:
	bool compileMetal(std::vector<std::uint8_t>& data, Output&, const std::string& metal) override
	{
		data.assign(metal.begin(), metal.end());
		return true;
	}

private:
	std::array<std::string, compile::stageCount> m_metalStrings;
};

} // namespace

using namespace compile;

TEST(TargetMetalTest, FragmentInputsOlderVersion)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";
	std::string shaderName = pathStr(inputDir/"CompleteFragmentInputShader.msl");

	MockTargetMetal target(202, TargetMetal::Platform::MacOS);

	Output output;
	CompiledResult result;
	EXPECT_FALSE(target.compile(result, output, shaderName));

	ASSERT_EQ(1U, output.getMessages().size());
	const std::vector<Output::Message>& messages = output.getMessages();
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(17U, messages[0].line);
	EXPECT_EQ(1U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("fragment inputs not supported by current target", messages[0].message);
}

TEST(TargetMetalTest, FragmentInputs)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";
	std::string shaderName = pathStr(inputDir/"CompleteFragmentInputShader.msl");

	MockTargetMetal target(203, TargetMetal::Platform::MacOS);

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
	std::size_t fragmentShaderIndex =
		pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader;
	ASSERT_EQ(1U, fragmentShaderIndex);
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
	EXPECT_EQ(3U, pipeline->second.fragmentOutputs[0].location);

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

	EXPECT_EQ(1U, pipeline->second.renderState.fragmentGroup);

	const CompiledResult::ShaderData& fragmentShader = result.getShaders()[fragmentShaderIndex];
	std::string fragmentShaderStr;
	fragmentShaderStr.assign(fragmentShader.data.begin(), fragmentShader.data.end());
	EXPECT_EQ(readFile(outputDir/"CompleteFragmentInputShader.frag.metal"), fragmentShaderStr);
}

TEST(TargetMetalTest, FragmentInputsStripDebug)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	boost::filesystem::path outputDir = exeDir/"outputs";
	std::string shaderName = pathStr(inputDir/"CompleteFragmentInputShader.msl");

	MockTargetMetal target(203, TargetMetal::Platform::MacOS);
	target.setStripDebug(true);

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
	std::size_t fragmentShaderIndex =
		pipeline->second.shaders[static_cast<int>(Stage::Fragment)].shader;
	ASSERT_EQ(1U, fragmentShaderIndex);
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
	EXPECT_EQ(3U, pipeline->second.fragmentOutputs[0].location);

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

	EXPECT_EQ(1U, pipeline->second.renderState.fragmentGroup);

	const CompiledResult::ShaderData& fragmentShader = result.getShaders()[fragmentShaderIndex];
	std::string fragmentShaderStr;
	fragmentShaderStr.assign(fragmentShader.data.begin(), fragmentShader.data.end());
	EXPECT_EQ(readFile(outputDir/"CompleteFragmentInputShaderStripped.frag.metal"),
		fragmentShaderStr);
}

} // namespace msl
