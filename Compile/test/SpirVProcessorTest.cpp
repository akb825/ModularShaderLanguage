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
#include "Compiler.h"
#include "Parser.h"
#include "Preprocessor.h"
#include "SpirVProcessor.h"
#include "StandAlone/ResourceLimits.h"
#include <gtest/gtest.h>

namespace msl
{

class SpirVProcessorTest : public testing::Test
{
public:
	void SetUp() override
	{
		Compiler::initialize();
	}

	void TearDown() override
	{
		Compiler::shutdown();
	}
};

TEST_F(SpirVProcessorTest, PrimitiveTypes)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"PrimitiveTypes.msl");

	Parser parser;
	Preprocessor preprocessor;
	Output output;
	EXPECT_TRUE(preprocessor.preprocess(parser.getTokens(), output, shaderName));
	EXPECT_TRUE(parser.parse(output));

	ASSERT_EQ(1U, parser.getPipelines().size());
	const Parser::Pipeline& pipeline = parser.getPipelines()[0];
	Compiler::Stages stages;
	bool compiledStage = false;
	for (unsigned int i = 0; i < stageCount; ++i)
	{
		if (pipeline.entryPoints[i].empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl = parser.createShaderString(lineMappings, pipeline, stage);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			glslang::DefaultTBuiltInResource));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	glslang::TProgram program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV spirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);
	EXPECT_TRUE(output.getMessages().empty());

	SpirVProcessor processor;
	EXPECT_TRUE(processor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, spirv, Stage::Fragment));

	ASSERT_EQ(1U, processor.structs.size());
	EXPECT_EQ("Uniforms", processor.structs[0].name);
	ASSERT_EQ(38U, processor.structs[0].members.size());

	EXPECT_EQ("Float", processor.structs[0].members[0].name);
	EXPECT_EQ(Type::Float, processor.structs[0].members[0].type);
	EXPECT_EQ(sizeof(float), processor.structs[0].members[0].size);

	EXPECT_EQ("Vec2", processor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec2, processor.structs[0].members[1].type);
	EXPECT_EQ(2*sizeof(float), processor.structs[0].members[1].size);

	EXPECT_EQ("Vec3", processor.structs[0].members[2].name);
	EXPECT_EQ(Type::Vec3, processor.structs[0].members[2].type);
	EXPECT_EQ(3*sizeof(float), processor.structs[0].members[2].size);

	EXPECT_EQ("Vec4", processor.structs[0].members[3].name);
	EXPECT_EQ(Type::Vec4, processor.structs[0].members[3].type);
	EXPECT_EQ(4*sizeof(float), processor.structs[0].members[3].size);

	EXPECT_EQ("Double", processor.structs[0].members[4].name);
	EXPECT_EQ(Type::Double, processor.structs[0].members[4].type);
	EXPECT_EQ(sizeof(double), processor.structs[0].members[4].size);

	EXPECT_EQ("DVec2", processor.structs[0].members[5].name);
	EXPECT_EQ(Type::DVec2, processor.structs[0].members[5].type);
	EXPECT_EQ(2*sizeof(double), processor.structs[0].members[5].size);

	EXPECT_EQ("DVec3", processor.structs[0].members[6].name);
	EXPECT_EQ(Type::DVec3, processor.structs[0].members[6].type);
	EXPECT_EQ(3*sizeof(double), processor.structs[0].members[6].size);

	EXPECT_EQ("DVec4", processor.structs[0].members[7].name);
	EXPECT_EQ(Type::DVec4, processor.structs[0].members[7].type);
	EXPECT_EQ(4*sizeof(double), processor.structs[0].members[7].size);

	EXPECT_EQ("Int", processor.structs[0].members[8].name);
	EXPECT_EQ(Type::Int, processor.structs[0].members[8].type);
	EXPECT_EQ(sizeof(int), processor.structs[0].members[8].size);

	EXPECT_EQ("IVec2", processor.structs[0].members[9].name);
	EXPECT_EQ(Type::IVec2, processor.structs[0].members[9].type);
	EXPECT_EQ(2*sizeof(int), processor.structs[0].members[9].size);

	EXPECT_EQ("IVec3", processor.structs[0].members[10].name);
	EXPECT_EQ(Type::IVec3, processor.structs[0].members[10].type);
	EXPECT_EQ(3*sizeof(int), processor.structs[0].members[10].size);

	EXPECT_EQ("IVec4", processor.structs[0].members[11].name);
	EXPECT_EQ(Type::IVec4, processor.structs[0].members[11].type);
	EXPECT_EQ(4*sizeof(int), processor.structs[0].members[11].size);

	EXPECT_EQ("UInt", processor.structs[0].members[12].name);
	EXPECT_EQ(Type::UInt, processor.structs[0].members[12].type);
	EXPECT_EQ(sizeof(int), processor.structs[0].members[12].size);

	EXPECT_EQ("UVec2", processor.structs[0].members[13].name);
	EXPECT_EQ(Type::UVec2, processor.structs[0].members[13].type);
	EXPECT_EQ(2*sizeof(int), processor.structs[0].members[13].size);

	EXPECT_EQ("UVec3", processor.structs[0].members[14].name);
	EXPECT_EQ(Type::UVec3, processor.structs[0].members[14].type);
	EXPECT_EQ(3*sizeof(int), processor.structs[0].members[14].size);

	EXPECT_EQ("UVec4", processor.structs[0].members[15].name);
	EXPECT_EQ(Type::UVec4, processor.structs[0].members[15].type);
	EXPECT_EQ(4*sizeof(int), processor.structs[0].members[15].size);

	// NOTE: glslang stores bools as uints.
	EXPECT_EQ("Bool", processor.structs[0].members[16].name);
	//EXPECT_EQ(Type::Bool, processor.structs[0].members[16].type);
	EXPECT_EQ(Type::UInt, processor.structs[0].members[16].type);
	EXPECT_EQ(sizeof(int), processor.structs[0].members[16].size);

	EXPECT_EQ("BVec2", processor.structs[0].members[17].name);
	//EXPECT_EQ(Type::BVec2, processor.structs[0].members[17].type);
	EXPECT_EQ(Type::UVec2, processor.structs[0].members[17].type);
	EXPECT_EQ(2*sizeof(int), processor.structs[0].members[17].size);

	EXPECT_EQ("BVec3", processor.structs[0].members[18].name);
	//EXPECT_EQ(Type::BVec3, processor.structs[0].members[18].type);
	EXPECT_EQ(Type::UVec3, processor.structs[0].members[18].type);
	EXPECT_EQ(3*sizeof(int), processor.structs[0].members[18].size);

	EXPECT_EQ("BVec4", processor.structs[0].members[19].name);
	//EXPECT_EQ(Type::BVec4, processor.structs[0].members[19].type);
	EXPECT_EQ(Type::UVec4, processor.structs[0].members[19].type);
	EXPECT_EQ(4*sizeof(int), processor.structs[0].members[19].size);

	EXPECT_EQ("Mat2", processor.structs[0].members[20].name);
	EXPECT_EQ(Type::Mat2, processor.structs[0].members[20].type);
	EXPECT_EQ(2*2*sizeof(float), processor.structs[0].members[20].size);

	EXPECT_EQ("Mat3", processor.structs[0].members[21].name);
	EXPECT_EQ(Type::Mat3, processor.structs[0].members[21].type);
	EXPECT_EQ(4*3*sizeof(float), processor.structs[0].members[21].size);

	EXPECT_EQ("Mat4", processor.structs[0].members[22].name);
	EXPECT_EQ(Type::Mat4, processor.structs[0].members[22].type);
	EXPECT_EQ(4*4*sizeof(float), processor.structs[0].members[22].size);

	EXPECT_EQ("Mat2x3", processor.structs[0].members[23].name);
	EXPECT_EQ(Type::Mat2x3, processor.structs[0].members[23].type);
	EXPECT_EQ(4*2*sizeof(float), processor.structs[0].members[23].size);

	EXPECT_EQ("Mat2x4", processor.structs[0].members[24].name);
	EXPECT_EQ(Type::Mat2x4, processor.structs[0].members[24].type);
	EXPECT_EQ(4*2*sizeof(float), processor.structs[0].members[24].size);

	EXPECT_EQ("Mat3x2", processor.structs[0].members[25].name);
	EXPECT_EQ(Type::Mat3x2, processor.structs[0].members[25].type);
	EXPECT_EQ(2*3*sizeof(float), processor.structs[0].members[25].size);

	EXPECT_EQ("Mat3x4", processor.structs[0].members[26].name);
	EXPECT_EQ(Type::Mat3x4, processor.structs[0].members[26].type);
	EXPECT_EQ(4*3*sizeof(float), processor.structs[0].members[26].size);

	EXPECT_EQ("Mat4x2", processor.structs[0].members[27].name);
	EXPECT_EQ(Type::Mat4x2, processor.structs[0].members[27].type);
	EXPECT_EQ(2*4*sizeof(float), processor.structs[0].members[27].size);

	EXPECT_EQ("Mat4x3", processor.structs[0].members[28].name);
	EXPECT_EQ(Type::Mat4x3, processor.structs[0].members[28].type);
	EXPECT_EQ(4*4*sizeof(float), processor.structs[0].members[28].size);

	EXPECT_EQ("DMat2", processor.structs[0].members[29].name);
	EXPECT_EQ(Type::DMat2, processor.structs[0].members[29].type);
	EXPECT_EQ(2*2*sizeof(double), processor.structs[0].members[29].size);

	EXPECT_EQ("DMat3", processor.structs[0].members[30].name);
	EXPECT_EQ(Type::DMat3, processor.structs[0].members[30].type);
	EXPECT_EQ(3*4*sizeof(double), processor.structs[0].members[30].size);

	EXPECT_EQ("DMat4", processor.structs[0].members[31].name);
	EXPECT_EQ(Type::DMat4, processor.structs[0].members[31].type);
	EXPECT_EQ(4*4*sizeof(double), processor.structs[0].members[31].size);

	EXPECT_EQ("DMat2x3", processor.structs[0].members[32].name);
	EXPECT_EQ(Type::DMat2x3, processor.structs[0].members[32].type);
	EXPECT_EQ(4*2*sizeof(double), processor.structs[0].members[32].size);

	EXPECT_EQ("DMat2x4", processor.structs[0].members[33].name);
	EXPECT_EQ(Type::DMat2x4, processor.structs[0].members[33].type);
	EXPECT_EQ(4*2*sizeof(double), processor.structs[0].members[33].size);

	EXPECT_EQ("DMat3x2", processor.structs[0].members[34].name);
	EXPECT_EQ(Type::DMat3x2, processor.structs[0].members[34].type);
	EXPECT_EQ(2*3*sizeof(double), processor.structs[0].members[34].size);

	EXPECT_EQ("DMat3x4", processor.structs[0].members[35].name);
	EXPECT_EQ(Type::DMat3x4, processor.structs[0].members[35].type);
	EXPECT_EQ(4*3*sizeof(double), processor.structs[0].members[35].size);

	EXPECT_EQ("DMat4x2", processor.structs[0].members[36].name);
	EXPECT_EQ(Type::DMat4x2, processor.structs[0].members[36].type);
	EXPECT_EQ(2*4*sizeof(double), processor.structs[0].members[36].size);

	EXPECT_EQ("DMat4x3", processor.structs[0].members[37].name);
	EXPECT_EQ(Type::DMat4x3, processor.structs[0].members[37].type);
	EXPECT_EQ(4*4*sizeof(double), processor.structs[0].members[37].size);

	ASSERT_EQ(68U, processor.uniforms.size());
	EXPECT_EQ("Sampler1D", processor.uniforms[0].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[0].uniformType);
	EXPECT_EQ(Type::Sampler1D, processor.uniforms[0].type);

	EXPECT_EQ("Sampler2D", processor.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, processor.uniforms[1].type);

	EXPECT_EQ("Sampler3D", processor.uniforms[2].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[2].uniformType);
	EXPECT_EQ(Type::Sampler3D, processor.uniforms[2].type);

	EXPECT_EQ("SamplerCube", processor.uniforms[3].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[3].uniformType);
	EXPECT_EQ(Type::SamplerCube, processor.uniforms[3].type);

	EXPECT_EQ("Sampler1DShadow", processor.uniforms[4].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[4].uniformType);
	EXPECT_EQ(Type::Sampler1DShadow, processor.uniforms[4].type);

	EXPECT_EQ("Sampler2DShadow", processor.uniforms[5].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[5].uniformType);
	EXPECT_EQ(Type::Sampler2DShadow, processor.uniforms[5].type);

	EXPECT_EQ("Sampler1DArray", processor.uniforms[6].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[6].uniformType);
	EXPECT_EQ(Type::Sampler1DArray, processor.uniforms[6].type);

	EXPECT_EQ("Sampler2DArray", processor.uniforms[7].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[7].uniformType);
	EXPECT_EQ(Type::Sampler2DArray, processor.uniforms[7].type);

	EXPECT_EQ("Sampler1DArrayShadow", processor.uniforms[8].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[8].uniformType);
	EXPECT_EQ(Type::Sampler1DArrayShadow, processor.uniforms[8].type);

	EXPECT_EQ("Sampler2DArrayShadow", processor.uniforms[9].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[9].uniformType);
	EXPECT_EQ(Type::Sampler2DArrayShadow, processor.uniforms[9].type);

	EXPECT_EQ("Sampler2DMS", processor.uniforms[10].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[10].uniformType);
	EXPECT_EQ(Type::Sampler2DMS, processor.uniforms[10].type);

	EXPECT_EQ("Sampler2DMSArray", processor.uniforms[11].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[11].uniformType);
	EXPECT_EQ(Type::Sampler2DMSArray, processor.uniforms[11].type);

	EXPECT_EQ("SamplerCubeShadow", processor.uniforms[12].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[12].uniformType);
	EXPECT_EQ(Type::SamplerCubeShadow, processor.uniforms[12].type);

	EXPECT_EQ("SamplerBuffer", processor.uniforms[13].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[13].uniformType);
	EXPECT_EQ(Type::SamplerBuffer, processor.uniforms[13].type);

	EXPECT_EQ("Sampler2DRect", processor.uniforms[14].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[14].uniformType);
	EXPECT_EQ(Type::Sampler2DRect, processor.uniforms[14].type);

	EXPECT_EQ("Sampler2DRectShadow", processor.uniforms[15].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[15].uniformType);
	EXPECT_EQ(Type::Sampler2DRectShadow, processor.uniforms[15].type);

	EXPECT_EQ("ISampler1D", processor.uniforms[16].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[16].uniformType);
	EXPECT_EQ(Type::ISampler1D, processor.uniforms[16].type);

	EXPECT_EQ("ISampler2D", processor.uniforms[17].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[17].uniformType);
	EXPECT_EQ(Type::ISampler2D, processor.uniforms[17].type);

	EXPECT_EQ("ISampler3D", processor.uniforms[18].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[18].uniformType);
	EXPECT_EQ(Type::ISampler3D, processor.uniforms[18].type);

	EXPECT_EQ("ISamplerCube", processor.uniforms[19].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[19].uniformType);
	EXPECT_EQ(Type::ISamplerCube, processor.uniforms[19].type);

	EXPECT_EQ("ISampler1DArray", processor.uniforms[20].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[20].uniformType);
	EXPECT_EQ(Type::ISampler1DArray, processor.uniforms[20].type);

	EXPECT_EQ("ISampler2DArray", processor.uniforms[21].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[21].uniformType);
	EXPECT_EQ(Type::ISampler2DArray, processor.uniforms[21].type);

	EXPECT_EQ("ISampler2DMS", processor.uniforms[22].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[22].uniformType);
	EXPECT_EQ(Type::ISampler2DMS, processor.uniforms[22].type);

	EXPECT_EQ("ISampler2DMSArray", processor.uniforms[23].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[23].uniformType);
	EXPECT_EQ(Type::ISampler2DMSArray, processor.uniforms[23].type);

	EXPECT_EQ("ISampler2DRect", processor.uniforms[24].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[24].uniformType);
	EXPECT_EQ(Type::ISampler2DRect, processor.uniforms[24].type);

	EXPECT_EQ("USampler1D", processor.uniforms[25].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[25].uniformType);
	EXPECT_EQ(Type::USampler1D, processor.uniforms[25].type);

	EXPECT_EQ("USampler2D", processor.uniforms[26].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[26].uniformType);
	EXPECT_EQ(Type::USampler2D, processor.uniforms[26].type);

	EXPECT_EQ("USampler3D", processor.uniforms[27].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[27].uniformType);
	EXPECT_EQ(Type::USampler3D, processor.uniforms[27].type);

	EXPECT_EQ("USamplerCube", processor.uniforms[28].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[28].uniformType);
	EXPECT_EQ(Type::USamplerCube, processor.uniforms[28].type);

	EXPECT_EQ("USampler1DArray", processor.uniforms[29].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[29].uniformType);
	EXPECT_EQ(Type::USampler1DArray, processor.uniforms[29].type);

	EXPECT_EQ("USampler2DArray", processor.uniforms[30].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[30].uniformType);
	EXPECT_EQ(Type::USampler2DArray, processor.uniforms[30].type);

	EXPECT_EQ("USampler2DMS", processor.uniforms[31].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[31].uniformType);
	EXPECT_EQ(Type::USampler2DMS, processor.uniforms[31].type);

	EXPECT_EQ("USampler2DMSArray", processor.uniforms[32].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[32].uniformType);
	EXPECT_EQ(Type::USampler2DMSArray, processor.uniforms[32].type);

	EXPECT_EQ("USampler2DRect", processor.uniforms[33].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[33].uniformType);
	EXPECT_EQ(Type::USampler2DRect, processor.uniforms[33].type);

	EXPECT_EQ("Image1D", processor.uniforms[34].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[34].uniformType);
	EXPECT_EQ(Type::Image1D, processor.uniforms[34].type);

	EXPECT_EQ("Image2D", processor.uniforms[35].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[35].uniformType);
	EXPECT_EQ(Type::Image2D, processor.uniforms[35].type);

	EXPECT_EQ("Image3D", processor.uniforms[36].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[36].uniformType);
	EXPECT_EQ(Type::Image3D, processor.uniforms[36].type);

	EXPECT_EQ("ImageCube", processor.uniforms[37].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[37].uniformType);
	EXPECT_EQ(Type::ImageCube, processor.uniforms[37].type);

	EXPECT_EQ("Image1DArray", processor.uniforms[38].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[38].uniformType);
	EXPECT_EQ(Type::Image1DArray, processor.uniforms[38].type);

	EXPECT_EQ("Image2DArray", processor.uniforms[39].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[39].uniformType);
	EXPECT_EQ(Type::Image2DArray, processor.uniforms[39].type);

	EXPECT_EQ("Image2DMS", processor.uniforms[40].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[40].uniformType);
	EXPECT_EQ(Type::Image2DMS, processor.uniforms[40].type);

	EXPECT_EQ("Image2DMSArray", processor.uniforms[41].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[41].uniformType);
	EXPECT_EQ(Type::Image2DMSArray, processor.uniforms[41].type);

	EXPECT_EQ("ImageBuffer", processor.uniforms[42].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[42].uniformType);
	EXPECT_EQ(Type::ImageBuffer, processor.uniforms[42].type);

	EXPECT_EQ("Image2DRect", processor.uniforms[43].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[43].uniformType);
	EXPECT_EQ(Type::Image2DRect, processor.uniforms[43].type);

	EXPECT_EQ("IImage1D", processor.uniforms[44].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[44].uniformType);
	EXPECT_EQ(Type::IImage1D, processor.uniforms[44].type);

	EXPECT_EQ("IImage2D", processor.uniforms[45].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[45].uniformType);
	EXPECT_EQ(Type::IImage2D, processor.uniforms[45].type);

	EXPECT_EQ("IImage3D", processor.uniforms[46].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[46].uniformType);
	EXPECT_EQ(Type::IImage3D, processor.uniforms[46].type);

	EXPECT_EQ("IImageCube", processor.uniforms[47].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[47].uniformType);
	EXPECT_EQ(Type::IImageCube, processor.uniforms[47].type);

	EXPECT_EQ("IImage1DArray", processor.uniforms[48].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[48].uniformType);
	EXPECT_EQ(Type::IImage1DArray, processor.uniforms[48].type);

	EXPECT_EQ("IImage2DArray", processor.uniforms[49].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[49].uniformType);
	EXPECT_EQ(Type::IImage2DArray, processor.uniforms[49].type);

	EXPECT_EQ("IImage2DMS", processor.uniforms[50].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[50].uniformType);
	EXPECT_EQ(Type::IImage2DMS, processor.uniforms[50].type);

	EXPECT_EQ("IImage2DMSArray", processor.uniforms[51].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[51].uniformType);
	EXPECT_EQ(Type::IImage2DMSArray, processor.uniforms[51].type);

	EXPECT_EQ("IImage2DRect", processor.uniforms[52].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[52].uniformType);
	EXPECT_EQ(Type::IImage2DRect, processor.uniforms[52].type);

	EXPECT_EQ("UImage1D", processor.uniforms[53].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[53].uniformType);
	EXPECT_EQ(Type::UImage1D, processor.uniforms[53].type);

	EXPECT_EQ("UImage2D", processor.uniforms[54].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[54].uniformType);
	EXPECT_EQ(Type::UImage2D, processor.uniforms[54].type);

	EXPECT_EQ("UImage3D", processor.uniforms[55].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[55].uniformType);
	EXPECT_EQ(Type::UImage3D, processor.uniforms[55].type);

	EXPECT_EQ("UImageCube", processor.uniforms[56].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[56].uniformType);
	EXPECT_EQ(Type::UImageCube, processor.uniforms[56].type);

	EXPECT_EQ("UImage1DArray", processor.uniforms[57].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[57].uniformType);
	EXPECT_EQ(Type::UImage1DArray, processor.uniforms[57].type);

	EXPECT_EQ("UImage2DArray", processor.uniforms[58].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[58].uniformType);
	EXPECT_EQ(Type::UImage2DArray, processor.uniforms[58].type);

	EXPECT_EQ("UImage2DMS", processor.uniforms[59].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[59].uniformType);
	EXPECT_EQ(Type::UImage2DMS, processor.uniforms[59].type);

	EXPECT_EQ("UImage2DMSArray", processor.uniforms[60].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[60].uniformType);
	EXPECT_EQ(Type::UImage2DMSArray, processor.uniforms[60].type);

	EXPECT_EQ("UImage2DRect", processor.uniforms[61].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[61].uniformType);
	EXPECT_EQ(Type::UImage2DRect, processor.uniforms[61].type);

	EXPECT_EQ("SubpassInput", processor.uniforms[62].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[62].uniformType);
	EXPECT_EQ(Type::SubpassInput, processor.uniforms[62].type);
	EXPECT_EQ(0U, processor.uniforms[62].binding);

	EXPECT_EQ("SubpassInputMS", processor.uniforms[63].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[63].uniformType);
	EXPECT_EQ(Type::SubpassInputMS, processor.uniforms[63].type);
	EXPECT_EQ(1U, processor.uniforms[63].binding);

	EXPECT_EQ("ISubpassInput", processor.uniforms[64].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[64].uniformType);
	EXPECT_EQ(Type::ISubpassInput, processor.uniforms[64].type);
	EXPECT_EQ(2U, processor.uniforms[64].binding);

	EXPECT_EQ("ISubpassInputMS", processor.uniforms[65].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[65].uniformType);
	EXPECT_EQ(Type::ISubpassInputMS, processor.uniforms[65].type);
	EXPECT_EQ(3U, processor.uniforms[65].binding);

	EXPECT_EQ("USubpassInput", processor.uniforms[66].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[66].uniformType);
	EXPECT_EQ(Type::USubpassInput, processor.uniforms[66].type);
	EXPECT_EQ(4U, processor.uniforms[66].binding);

	EXPECT_EQ("USubpassInputMS", processor.uniforms[67].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[67].uniformType);
	EXPECT_EQ(Type::USubpassInputMS, processor.uniforms[67].type);
	EXPECT_EQ(5U, processor.uniforms[67].binding);
}

} // namespace msl
