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
	auto version = Module::currentVersion;
	EXPECT_EQ(version, module.version());
	EXPECT_EQ(MSL_CREATE_ID('S', 'P', 'R', 'V'), module.targetId());
	EXPECT_EQ(1U, module.targetVersion());

	auto noShader = Module::noShader;
	auto unknown = Module::unknown;
	ASSERT_EQ(1U, module.pipelineCount());
	EXPECT_STREQ("Test", module.pipelineName(0));
	EXPECT_EQ(0U, module.pipelineShader(0, Module::Stage::Vertex));
	EXPECT_EQ(noShader, module.pipelineShader(0, Module::Stage::TessellationControl));
	EXPECT_EQ(noShader, module.pipelineShader(0, Module::Stage::TessellationEvaluation));
	EXPECT_EQ(noShader, module.pipelineShader(0, Module::Stage::Geometry));
	EXPECT_EQ(1U, module.pipelineShader(0, Module::Stage::Fragment));
	EXPECT_EQ(noShader, module.pipelineShader(0, Module::Stage::Compute));

	ASSERT_EQ(2U, module.uniformCount(0));
	EXPECT_STREQ("Transform.transform", module.uniformName(0, 0));
	EXPECT_EQ(Module::Type::Mat4, module.uniformType(0, 0));
	EXPECT_EQ(0U, module.uniformBlockIndex(0, 0));
	EXPECT_EQ(0U, module.uniformBufferOffset(0, 0));
	EXPECT_EQ(1U, module.uniformElements(0, 0));

	EXPECT_STREQ("tex", module.uniformName(0, 1));
	EXPECT_EQ(Module::Type::Sampler2D, module.uniformType(0, 1));
	EXPECT_EQ(unknown, module.uniformBlockIndex(0, 1));
	EXPECT_EQ(unknown, module.uniformBufferOffset(0, 1));
	EXPECT_EQ(1U, module.uniformElements(0, 1));

	ASSERT_EQ(1U, module.uniformBlockCount(0));
	EXPECT_STREQ("Transform", module.uniformBlockName(0, 0));
	EXPECT_EQ(sizeof(float)*16, module.uniformBlockSize(0, 0));

	ASSERT_EQ(2U, module.attributeCount(0));
	EXPECT_STREQ("position", module.attributeName(0, 0));
	EXPECT_EQ(Module::Type::Vec3, module.attributeType(0, 0));

	EXPECT_STREQ("color", module.attributeName(0, 1));
	EXPECT_EQ(Module::Type::Vec4, module.attributeType(0, 1));

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
	EXPECT_STREQ("Test", mslModule_pipelineName(module, 0));
	EXPECT_EQ(0U, mslModule_pipelineShader(module, 0, mslStage_Vertex));
	EXPECT_EQ(MSL_NO_SHADER, mslModule_pipelineShader(module, 0, mslStage_TessellationControl));
	EXPECT_EQ(MSL_NO_SHADER, mslModule_pipelineShader(module, 0, mslStage_TessellationEvaluation));
	EXPECT_EQ(MSL_NO_SHADER, mslModule_pipelineShader(module, 0, mslStage_Geometry));
	EXPECT_EQ(1U, mslModule_pipelineShader(module, 0, mslStage_Fragment));
	EXPECT_EQ(MSL_NO_SHADER, mslModule_pipelineShader(module, 0, mslStage_Compute));

	ASSERT_EQ(2U, mslModule_uniformCount(module, 0));
	EXPECT_STREQ("Transform.transform", mslModule_uniformName(module, 0, 0));
	EXPECT_EQ(mslType_Mat4, mslModule_uniformType(module, 0, 0));
	EXPECT_EQ(0U, mslModule_uniformBlockIndex(module, 0, 0));
	EXPECT_EQ(0U, mslModule_uniformBufferOffset(module, 0, 0));
	EXPECT_EQ(1U, mslModule_uniformElements(module, 0, 0));

	EXPECT_STREQ("tex", mslModule_uniformName(module, 0, 1));
	EXPECT_EQ(mslType_Sampler2D, mslModule_uniformType(module, 0, 1));
	EXPECT_EQ(MSL_UNKNOWN, mslModule_uniformBlockIndex(module, 0, 1));
	EXPECT_EQ(MSL_UNKNOWN, mslModule_uniformBufferOffset(module, 0, 1));
	EXPECT_EQ(1U, mslModule_uniformElements(module, 0, 1));

	ASSERT_EQ(1U, mslModule_uniformBlockCount(module, 0));
	EXPECT_STREQ("Transform", mslModule_uniformBlockName(module, 0, 0));
	EXPECT_EQ(sizeof(float)*16, mslModule_uniformBlockSize(module, 0, 0));

	ASSERT_EQ(2U, mslModule_attributeCount(module, 0));
	EXPECT_STREQ("position", mslModule_attributeName(module, 0, 0));
	EXPECT_EQ(mslType_Vec3, mslModule_attributeType(module, 0, 0));

	EXPECT_STREQ("color", mslModule_attributeName(module, 0, 1));
	EXPECT_EQ(mslType_Vec4, mslModule_attributeType(module, 0, 1));

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
	std::string fileName = (exeDir/"CompleteShader.mslb").string();
	Module module;
	EXPECT_TRUE(module.read(fileName));
	testContents(module);
}

TEST(ModuleTest, ReadData)
{
	std::string fileName = (exeDir/"CompleteShader.mslb").string();
	std::ifstream stream(fileName);
	std::vector<std::uint8_t> data(std::istreambuf_iterator<char>(stream.rdbuf()),
		std::istreambuf_iterator<char>());

	Module module;
	EXPECT_TRUE(module.read(data.data(), data.size()));
	testContents(module);
}

TEST(ModuleTest, ReadInvalidData)
{
	std::string fileName = (exeDir/"CompleteShader.mslb").string();
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
	std::string fileName = (exeDir/"CompleteShader.mslb").string();
	mslModule* module = mslModule_readFile(fileName.c_str(), nullptr);
	testContents(module);
	mslModule_destroy(module);
}

TEST(ModuleTest, InvalidAllocator)
{
	mslAllocator allocator = {};
	std::string fileName = (exeDir/"CompleteShader.mslb").string();
	EXPECT_EQ(nullptr, mslModule_readFile(fileName.c_str(), &allocator));
	EXPECT_EQ(EINVAL, errno);
}

} // namespace msl
