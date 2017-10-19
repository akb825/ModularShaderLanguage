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

#include <MSL/Client/ModuleCpp.h>
#include "Helpers.h"
#include <gtest/gtest.h>
#include <fstream>
#include <errno.h>

namespace msl
{

static void testContents(Module& module)
{
	EXPECT_EQ(moduleVersion, module.version());
	EXPECT_EQ(MSL_CREATE_ID('S', 'P', 'R', 'V'), module.targetId());
	EXPECT_EQ(1U, module.targetVersion());

	ASSERT_EQ(1U, module.pipelineCount());
	Pipeline pipeline;
	EXPECT_TRUE(module.pipeline(pipeline, 0));
	EXPECT_STREQ("Test", pipeline.name);
	EXPECT_EQ(0U, pipeline.shaders[static_cast<int>(Stage::Vertex)]);
	EXPECT_EQ(unknown, pipeline.shaders[static_cast<int>(Stage::TessellationControl)]);
	EXPECT_EQ(unknown, pipeline.shaders[static_cast<int>(Stage::TessellationEvaluation)]);
	EXPECT_EQ(unknown, pipeline.shaders[static_cast<int>(Stage::Geometry)]);
	EXPECT_EQ(1U, pipeline.shaders[static_cast<int>(Stage::Fragment)]);
	EXPECT_EQ(unknown, pipeline.shaders[static_cast<int>(Stage::Compute)]);

	Struct pipelineStruct;
	ASSERT_EQ(2U, pipeline.structCount);
	EXPECT_TRUE(module.pipelineStruct(pipelineStruct, 0, 0));
	EXPECT_STREQ("Uniforms", pipelineStruct.name);
	EXPECT_EQ(4*sizeof(float), pipelineStruct.size);

	StructMember structMember;
	ASSERT_EQ(1U, pipelineStruct.memberCount);
	EXPECT_TRUE(module.structMember(structMember, 0, 0, 0));
	EXPECT_STREQ("texCoords", structMember.name);
	EXPECT_EQ(0U, structMember.offset);
	EXPECT_EQ(4*sizeof(float), structMember.size);
	EXPECT_EQ(Type::Vec2, structMember.type);
	EXPECT_EQ(unknown, structMember.structIndex);
	ASSERT_EQ(1U, structMember.arrayElementCount);
	EXPECT_FALSE(structMember.rowMajor);

	ArrayInfo arrayInfo;
	EXPECT_TRUE(module.structMemberArrayInfo(arrayInfo, 0, 0, 0, 0));
	EXPECT_EQ(2U, arrayInfo.length);
	EXPECT_EQ(2*sizeof(float), arrayInfo.stride);

	EXPECT_TRUE(module.pipelineStruct(pipelineStruct, 0, 1));
	EXPECT_STREQ("Transform", pipelineStruct.name);
	EXPECT_EQ(16*sizeof(float), pipelineStruct.size);

	ASSERT_EQ(1U, pipelineStruct.memberCount);
	EXPECT_TRUE(module.structMember(structMember, 0, 1, 0));
	EXPECT_STREQ("transform", structMember.name);
	EXPECT_EQ(0U, structMember.offset);
	EXPECT_EQ(16*sizeof(float), structMember.size);
	EXPECT_EQ(Type::Mat4, structMember.type);
	EXPECT_EQ(unknown, structMember.structIndex);
	EXPECT_EQ(0U, structMember.arrayElementCount);
	EXPECT_FALSE(structMember.rowMajor);

	Uniform uniform;
	ASSERT_EQ(3U, pipeline.uniformCount);
	EXPECT_TRUE(module.uniform(uniform, 0, 0));
	EXPECT_STREQ("Uniforms", uniform.name);
	EXPECT_EQ(UniformType::PushConstant, uniform.uniformType);
	EXPECT_EQ(Type::Struct, uniform.type);
	EXPECT_EQ(0U, uniform.structIndex);
	EXPECT_EQ(0U, uniform.arrayElementCount);
	EXPECT_EQ(unknown, uniform.descriptorSet);
	EXPECT_EQ(unknown, uniform.binding);
	EXPECT_EQ(unknown, uniform.inputAttachmentIndex);
	EXPECT_EQ(unknown, uniform.samplerIndex);

	EXPECT_TRUE(module.uniform(uniform, 0, 1));
	EXPECT_STREQ("Transform", uniform.name);
	EXPECT_EQ(UniformType::Block, uniform.uniformType);
	EXPECT_EQ(Type::Struct, uniform.type);
	EXPECT_EQ(1U, uniform.structIndex);
	EXPECT_EQ(0U, uniform.arrayElementCount);
	EXPECT_EQ(0U, uniform.descriptorSet);
	EXPECT_EQ(unknown, uniform.binding);
	EXPECT_EQ(unknown, uniform.inputAttachmentIndex);
	EXPECT_EQ(unknown, uniform.samplerIndex);

	EXPECT_TRUE(module.uniform(uniform, 0, 2));
	EXPECT_STREQ("tex", uniform.name);
	EXPECT_EQ(UniformType::SampledImage, uniform.uniformType);
	EXPECT_EQ(Type::Sampler2D, uniform.type);
	EXPECT_EQ(unknown, uniform.structIndex);
	EXPECT_EQ(0U, uniform.arrayElementCount);
	EXPECT_EQ(0U, uniform.descriptorSet);
	EXPECT_EQ(unknown, uniform.binding);
	EXPECT_EQ(unknown, uniform.inputAttachmentIndex);
	EXPECT_EQ(0U, uniform.samplerIndex);

	Attribute attribute;
	ASSERT_EQ(2U, pipeline.attributeCount);
	EXPECT_TRUE(module.attribute(attribute, 0, 0));
	EXPECT_STREQ("position", attribute.name);
	EXPECT_EQ(Type::Vec3, attribute.type);
	EXPECT_EQ(0U, attribute.arrayElementCount);
	EXPECT_EQ(0U, attribute.location);
	EXPECT_EQ(0U, attribute.component);

	EXPECT_TRUE(module.attribute(attribute, 0, 1));
	EXPECT_STREQ("color", attribute.name);
	EXPECT_EQ(Type::Vec4, attribute.type);
	EXPECT_EQ(0U, attribute.arrayElementCount);
	EXPECT_EQ(1U, attribute.location);
	EXPECT_EQ(0U, attribute.component);

	FragmentOutput fragmentOutput;
	ASSERT_EQ(1U, pipeline.fragmentOutputCount);
	EXPECT_TRUE(module.fragmentOutput(fragmentOutput, 0, 0));
	EXPECT_STREQ("color", fragmentOutput.name);
	EXPECT_EQ(0U, fragmentOutput.location);

	SamplerState samplerState;
	ASSERT_EQ(1U, pipeline.samplerStateCount);
	EXPECT_TRUE(module.samplerState(samplerState, 0, 0));
	EXPECT_EQ(Filter::Linear, samplerState.minFilter);
	EXPECT_EQ(Filter::Linear, samplerState.magFilter);
	EXPECT_EQ(MipFilter::Anisotropic, samplerState.mipFilter);
	EXPECT_EQ(AddressMode::Repeat, samplerState.addressModeU);
	EXPECT_EQ(AddressMode::ClampToEdge, samplerState.addressModeV);
	EXPECT_EQ(AddressMode::Unset, samplerState.addressModeW);
	EXPECT_EQ(unknownFloat, samplerState.mipLodBias);
	EXPECT_EQ(unknownFloat, samplerState.maxAnisotropy);
	EXPECT_EQ(unknownFloat, samplerState.minLod);
	EXPECT_EQ(unknownFloat, samplerState.maxLod);
	EXPECT_EQ(BorderColor::Unset, samplerState.borderColor);

	RenderState renderState;
	EXPECT_TRUE(module.renderState(renderState, 0));
	EXPECT_EQ(msl::Bool::True, renderState.blendState.blendAttachments[0].blendEnable);
	EXPECT_EQ(msl::BlendFactor::SrcAlpha,
		renderState.blendState.blendAttachments[0].srcColorBlendFactor);
	EXPECT_EQ(msl::BlendFactor::SrcAlpha,
		renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	EXPECT_EQ(msl::BlendFactor::OneMinusSrcAlpha,
		renderState.blendState.blendAttachments[0].dstColorBlendFactor);
	EXPECT_EQ(msl::BlendFactor::Zero,
		renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);

	ASSERT_EQ(2U, module.shaderCount());
	EXPECT_LT(0U, module.shaderSize(0));
	EXPECT_NE(nullptr, module.shaderData(0));

	EXPECT_LT(0U, module.shaderSize(1));
	EXPECT_NE(nullptr, module.shaderData(1));

	EXPECT_EQ(0U, module.sharedDataSize());
	EXPECT_NE(nullptr, module.sharedData());
}

static void testContents(const mslModule* module)
{
	EXPECT_EQ(MSL_MODULE_VERSION, mslModule_version(module));
	EXPECT_EQ(MSL_CREATE_ID('S', 'P', 'R', 'V'), mslModule_targetId(module));
	EXPECT_EQ(1U, mslModule_targetVersion(module));

	ASSERT_EQ(1U, mslModule_pipelineCount(module));
	mslPipeline pipeline;
	EXPECT_TRUE(mslModule_pipeline(&pipeline, module, 0));
	EXPECT_STREQ("Test", pipeline.name);
	EXPECT_EQ(0U, pipeline.shaders[mslStage_Vertex]);
	EXPECT_EQ(MSL_UNKNOWN, pipeline.shaders[mslStage_TessellationControl]);
	EXPECT_EQ(MSL_UNKNOWN, pipeline.shaders[mslStage_TessellationEvaluation]);
	EXPECT_EQ(MSL_UNKNOWN, pipeline.shaders[mslStage_Geometry]);
	EXPECT_EQ(1U, pipeline.shaders[mslStage_Fragment]);
	EXPECT_EQ(unknown, pipeline.shaders[mslStage_Compute]);

	mslStruct pipelineStruct;
	ASSERT_EQ(2U, pipeline.structCount);
	EXPECT_TRUE(mslModule_struct(&pipelineStruct, module, 0, 0));
	EXPECT_STREQ("Uniforms", pipelineStruct.name);
	EXPECT_EQ(4*sizeof(float), pipelineStruct.size);

	mslStructMember structMember;
	ASSERT_EQ(1U, pipelineStruct.memberCount);
	EXPECT_TRUE(mslModule_structMember(&structMember, module, 0, 0, 0));
	EXPECT_STREQ("texCoords", structMember.name);
	EXPECT_EQ(0U, structMember.offset);
	EXPECT_EQ(4*sizeof(float), structMember.size);
	EXPECT_EQ(mslType_Vec2, structMember.type);
	EXPECT_EQ(MSL_UNKNOWN, structMember.structIndex);
	ASSERT_EQ(1U, structMember.arrayElementCount);
	EXPECT_FALSE(structMember.rowMajor);

	EXPECT_TRUE(mslModule_struct(&pipelineStruct, module, 0, 1));
	EXPECT_STREQ("Transform", pipelineStruct.name);
	EXPECT_EQ(16*sizeof(float), pipelineStruct.size);

	ASSERT_EQ(1U, pipelineStruct.memberCount);
	EXPECT_TRUE(mslModule_structMember(&structMember, module, 0, 1, 0));
	EXPECT_STREQ("transform", structMember.name);
	EXPECT_EQ(0U, structMember.offset);
	EXPECT_EQ(16*sizeof(float), structMember.size);
	EXPECT_EQ(mslType_Mat4, structMember.type);
	EXPECT_EQ(MSL_UNKNOWN, structMember.structIndex);
	EXPECT_EQ(0U, structMember.arrayElementCount);
	EXPECT_FALSE(structMember.rowMajor);

	mslUniform uniform;
	ASSERT_EQ(3U, pipeline.uniformCount);
	EXPECT_TRUE(mslModule_uniform(&uniform, module, 0, 0));
	EXPECT_STREQ("Uniforms", uniform.name);
	EXPECT_EQ(mslUniformType_PushConstant, uniform.uniformType);
	EXPECT_EQ(mslType_Struct, uniform.type);
	EXPECT_EQ(0U, uniform.structIndex);
	EXPECT_EQ(0U, uniform.arrayElementCount);
	EXPECT_EQ(MSL_UNKNOWN, uniform.descriptorSet);
	EXPECT_EQ(MSL_UNKNOWN, uniform.binding);
	EXPECT_EQ(MSL_UNKNOWN, uniform.inputAttachmentIndex);
	EXPECT_EQ(MSL_UNKNOWN, uniform.samplerIndex);

	EXPECT_TRUE(mslModule_uniform(&uniform, module, 0, 1));
	EXPECT_STREQ("Transform", uniform.name);
	EXPECT_EQ(mslUniformType_Block, uniform.uniformType);
	EXPECT_EQ(mslType_Struct, uniform.type);
	EXPECT_EQ(1U, uniform.structIndex);
	EXPECT_EQ(0U, uniform.arrayElementCount);
	EXPECT_EQ(0U, uniform.descriptorSet);
	EXPECT_EQ(MSL_UNKNOWN, uniform.binding);
	EXPECT_EQ(MSL_UNKNOWN, uniform.inputAttachmentIndex);
	EXPECT_EQ(MSL_UNKNOWN, uniform.samplerIndex);

	EXPECT_TRUE(mslModule_uniform(&uniform, module, 0, 2));
	EXPECT_STREQ("tex", uniform.name);
	EXPECT_EQ(mslUniformType_SampledImage, uniform.uniformType);
	EXPECT_EQ(mslType_Sampler2D, uniform.type);
	EXPECT_EQ(MSL_UNKNOWN, uniform.structIndex);
	EXPECT_EQ(0U, uniform.arrayElementCount);
	EXPECT_EQ(0U, uniform.descriptorSet);
	EXPECT_EQ(MSL_UNKNOWN, uniform.binding);
	EXPECT_EQ(MSL_UNKNOWN, uniform.inputAttachmentIndex);
	EXPECT_EQ(0U, uniform.samplerIndex);

	mslAttribute attribute;
	ASSERT_EQ(2U, pipeline.attributeCount);
	EXPECT_TRUE(mslModule_attribute(&attribute, module, 0, 0));
	EXPECT_STREQ("position", attribute.name);
	EXPECT_EQ(mslType_Vec3, attribute.type);
	EXPECT_EQ(0U, attribute.arrayElementCount);
	EXPECT_EQ(0U, attribute.location);
	EXPECT_EQ(0U, attribute.component);

	EXPECT_TRUE(mslModule_attribute(&attribute, module, 0, 1));
	EXPECT_STREQ("color", attribute.name);
	EXPECT_EQ(mslType_Vec4, attribute.type);
	EXPECT_EQ(0U, attribute.arrayElementCount);
	EXPECT_EQ(1U, attribute.location);
	EXPECT_EQ(0U, attribute.component);

	mslFragmentOutput fragmentOutput;
	ASSERT_EQ(1U, pipeline.fragmentOutputCount);
	EXPECT_TRUE(mslModule_fragmentOutput(&fragmentOutput, module, 0, 0));
	EXPECT_STREQ("color", fragmentOutput.name);
	EXPECT_EQ(0U, fragmentOutput.location);

	mslSamplerState samplerState;
	ASSERT_EQ(1U, pipeline.samplerStateCount);
	EXPECT_TRUE(mslModule_samplerState(&samplerState, module, 0, 0));
	EXPECT_EQ(mslFilter_Linear, samplerState.minFilter);
	EXPECT_EQ(mslFilter_Linear, samplerState.magFilter);
	EXPECT_EQ(mslMipFilter_Anisotropic, samplerState.mipFilter);
	EXPECT_EQ(mslAddressMode_Repeat, samplerState.addressModeU);
	EXPECT_EQ(mslAddressMode_ClampToEdge, samplerState.addressModeV);
	EXPECT_EQ(mslAddressMode_Unset, samplerState.addressModeW);
	EXPECT_EQ(MSL_UNKNOWN_FLOAT, samplerState.mipLodBias);
	EXPECT_EQ(MSL_UNKNOWN_FLOAT, samplerState.maxAnisotropy);
	EXPECT_EQ(MSL_UNKNOWN_FLOAT, samplerState.minLod);
	EXPECT_EQ(MSL_UNKNOWN_FLOAT, samplerState.maxLod);
	EXPECT_EQ(mslBorderColor_Unset, samplerState.borderColor);
	EXPECT_EQ(mslCompareOp_Unset, samplerState.compareOp);

	mslRenderState renderState;
	EXPECT_TRUE(mslModule_renderState(&renderState, module, 0));
	EXPECT_EQ(mslBool_True, renderState.blendState.blendAttachments[0].blendEnable);
	EXPECT_EQ(mslBlendFactor_SrcAlpha,
		renderState.blendState.blendAttachments[0].srcColorBlendFactor);
	EXPECT_EQ(mslBlendFactor_SrcAlpha,
		renderState.blendState.blendAttachments[0].srcAlphaBlendFactor);
	EXPECT_EQ(mslBlendFactor_OneMinusSrcAlpha,
		renderState.blendState.blendAttachments[0].dstColorBlendFactor);
	EXPECT_EQ(mslBlendFactor_Zero,
		renderState.blendState.blendAttachments[0].dstAlphaBlendFactor);

	ASSERT_EQ(2U, mslModule_shaderCount(module));
	EXPECT_LT(0U, mslModule_shaderSize(module, 0));
	EXPECT_NE(nullptr, mslModule_shaderData(module, 0));

	EXPECT_LT(0U, mslModule_shaderSize(module, 1));
	EXPECT_NE(nullptr, mslModule_shaderData(module, 1));

	EXPECT_EQ(0U, mslModule_sharedDataSize(module));
	EXPECT_NE(nullptr, mslModule_sharedData(module));
}

TEST(ModuleTest, ReadFile)
{
	std::string fileName = pathStr(exeDir/"CompleteShader.mslb");
	Module module;
	EXPECT_TRUE(module.read(fileName));
	testContents(module);
}

TEST(ModuleTest, ReadData)
{
	std::string fileName = pathStr(exeDir/"CompleteShader.mslb");
	std::ifstream stream(fileName, std::ios_base::binary);
	std::vector<std::uint8_t> data(std::istreambuf_iterator<char>(stream.rdbuf()),
		std::istreambuf_iterator<char>());

	Module module;
	EXPECT_TRUE(module.read(data.data(), data.size()));
	testContents(module);
}

TEST(ModuleTest, ReadInvalidData)
{
	std::string fileName = pathStr(exeDir/"CompleteShader.mslb");
	std::ifstream stream(fileName);
	std::vector<std::uint8_t> data(std::istreambuf_iterator<char>(stream.rdbuf()),
		std::istreambuf_iterator<char>());

	Module module;
	EXPECT_FALSE(module.read(data.data(), data.size() - 18));
	EXPECT_EQ(EILSEQ, errno);

	stream.seekg(0);
	EXPECT_FALSE(module.read(stream, data.size() + 18));
	EXPECT_EQ(EIO, errno);
}

TEST(ModuleTest, ReadFileC)
{
	std::string fileName = pathStr(exeDir/"CompleteShader.mslb");
	mslModule* module = mslModule_readFile(fileName.c_str(), nullptr);
	testContents(module);
	mslModule_destroy(module);
}

TEST(ModuleTest, InvalidAllocator)
{
	mslAllocator allocator = {};
	std::string fileName = pathStr(exeDir/"CompleteShader.mslb");
	EXPECT_EQ(nullptr, mslModule_readFile(fileName.c_str(), &allocator));
	EXPECT_EQ(EINVAL, errno);
}

} // namespace msl
