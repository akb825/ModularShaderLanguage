/*
 * Copyright 2016-2025 Aaron Barany
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
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>

namespace msl
{

namespace compile
{

inline bool operator==(const ArrayInfo& p1, const ArrayInfo& p2)
{
	return p1.length == p2.length && p1.stride == p2.stride;
}

} // namespace compile

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

static constexpr std::uint32_t spirvVersion = 0x10600;

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV spirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor processor;
	EXPECT_TRUE(processor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, spirv, Stage::Fragment));
	EXPECT_TRUE(output.getMessages().empty());

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

	ASSERT_EQ(69U, processor.uniforms.size());
	EXPECT_EQ("Uniforms", processor.uniforms[0].name);
	EXPECT_EQ(UniformType::PushConstant, processor.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, processor.uniforms[0].type);
	EXPECT_EQ(0U, processor.uniforms[0].structIndex);

	EXPECT_EQ("Sampler1D", processor.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler1D, processor.uniforms[1].type);

	EXPECT_EQ("Sampler2D", processor.uniforms[2].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[2].uniformType);
	EXPECT_EQ(Type::Sampler2D, processor.uniforms[2].type);

	EXPECT_EQ("Sampler3D", processor.uniforms[3].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[3].uniformType);
	EXPECT_EQ(Type::Sampler3D, processor.uniforms[3].type);

	EXPECT_EQ("SamplerCube", processor.uniforms[4].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[4].uniformType);
	EXPECT_EQ(Type::SamplerCube, processor.uniforms[4].type);

	EXPECT_EQ("Sampler1DShadow", processor.uniforms[5].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[5].uniformType);
	EXPECT_EQ(Type::Sampler1DShadow, processor.uniforms[5].type);

	EXPECT_EQ("Sampler2DShadow", processor.uniforms[6].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[6].uniformType);
	EXPECT_EQ(Type::Sampler2DShadow, processor.uniforms[6].type);

	EXPECT_EQ("Sampler1DArray", processor.uniforms[7].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[7].uniformType);
	EXPECT_EQ(Type::Sampler1DArray, processor.uniforms[7].type);

	EXPECT_EQ("Sampler2DArray", processor.uniforms[8].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[8].uniformType);
	EXPECT_EQ(Type::Sampler2DArray, processor.uniforms[8].type);

	EXPECT_EQ("Sampler1DArrayShadow", processor.uniforms[9].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[9].uniformType);
	EXPECT_EQ(Type::Sampler1DArrayShadow, processor.uniforms[9].type);

	EXPECT_EQ("Sampler2DArrayShadow", processor.uniforms[10].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[10].uniformType);
	EXPECT_EQ(Type::Sampler2DArrayShadow, processor.uniforms[10].type);

	EXPECT_EQ("Sampler2DMS", processor.uniforms[11].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[11].uniformType);
	EXPECT_EQ(Type::Sampler2DMS, processor.uniforms[11].type);

	EXPECT_EQ("Sampler2DMSArray", processor.uniforms[12].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[12].uniformType);
	EXPECT_EQ(Type::Sampler2DMSArray, processor.uniforms[12].type);

	EXPECT_EQ("SamplerCubeShadow", processor.uniforms[13].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[13].uniformType);
	EXPECT_EQ(Type::SamplerCubeShadow, processor.uniforms[13].type);

	EXPECT_EQ("SamplerBuffer", processor.uniforms[14].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[14].uniformType);
	EXPECT_EQ(Type::SamplerBuffer, processor.uniforms[14].type);

	EXPECT_EQ("Sampler2DRect", processor.uniforms[15].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[15].uniformType);
	EXPECT_EQ(Type::Sampler2DRect, processor.uniforms[15].type);

	EXPECT_EQ("Sampler2DRectShadow", processor.uniforms[16].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[16].uniformType);
	EXPECT_EQ(Type::Sampler2DRectShadow, processor.uniforms[16].type);

	EXPECT_EQ("ISampler1D", processor.uniforms[17].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[17].uniformType);
	EXPECT_EQ(Type::ISampler1D, processor.uniforms[17].type);

	EXPECT_EQ("ISampler2D", processor.uniforms[18].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[18].uniformType);
	EXPECT_EQ(Type::ISampler2D, processor.uniforms[18].type);

	EXPECT_EQ("ISampler3D", processor.uniforms[19].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[19].uniformType);
	EXPECT_EQ(Type::ISampler3D, processor.uniforms[19].type);

	EXPECT_EQ("ISamplerCube", processor.uniforms[20].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[20].uniformType);
	EXPECT_EQ(Type::ISamplerCube, processor.uniforms[20].type);

	EXPECT_EQ("ISampler1DArray", processor.uniforms[21].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[21].uniformType);
	EXPECT_EQ(Type::ISampler1DArray, processor.uniforms[21].type);

	EXPECT_EQ("ISampler2DArray", processor.uniforms[22].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[22].uniformType);
	EXPECT_EQ(Type::ISampler2DArray, processor.uniforms[22].type);

	EXPECT_EQ("ISampler2DMS", processor.uniforms[23].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[23].uniformType);
	EXPECT_EQ(Type::ISampler2DMS, processor.uniforms[23].type);

	EXPECT_EQ("ISampler2DMSArray", processor.uniforms[24].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[24].uniformType);
	EXPECT_EQ(Type::ISampler2DMSArray, processor.uniforms[24].type);

	EXPECT_EQ("ISampler2DRect", processor.uniforms[25].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[25].uniformType);
	EXPECT_EQ(Type::ISampler2DRect, processor.uniforms[25].type);

	EXPECT_EQ("USampler1D", processor.uniforms[26].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[26].uniformType);
	EXPECT_EQ(Type::USampler1D, processor.uniforms[26].type);

	EXPECT_EQ("USampler2D", processor.uniforms[27].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[27].uniformType);
	EXPECT_EQ(Type::USampler2D, processor.uniforms[27].type);

	EXPECT_EQ("USampler3D", processor.uniforms[28].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[28].uniformType);
	EXPECT_EQ(Type::USampler3D, processor.uniforms[28].type);

	EXPECT_EQ("USamplerCube", processor.uniforms[29].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[29].uniformType);
	EXPECT_EQ(Type::USamplerCube, processor.uniforms[29].type);

	EXPECT_EQ("USampler1DArray", processor.uniforms[30].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[30].uniformType);
	EXPECT_EQ(Type::USampler1DArray, processor.uniforms[30].type);

	EXPECT_EQ("USampler2DArray", processor.uniforms[31].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[31].uniformType);
	EXPECT_EQ(Type::USampler2DArray, processor.uniforms[31].type);

	EXPECT_EQ("USampler2DMS", processor.uniforms[32].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[32].uniformType);
	EXPECT_EQ(Type::USampler2DMS, processor.uniforms[32].type);

	EXPECT_EQ("USampler2DMSArray", processor.uniforms[33].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[33].uniformType);
	EXPECT_EQ(Type::USampler2DMSArray, processor.uniforms[33].type);

	EXPECT_EQ("USampler2DRect", processor.uniforms[34].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[34].uniformType);
	EXPECT_EQ(Type::USampler2DRect, processor.uniforms[34].type);

	EXPECT_EQ("Image1D", processor.uniforms[35].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[35].uniformType);
	EXPECT_EQ(Type::Image1D, processor.uniforms[35].type);

	EXPECT_EQ("Image2D", processor.uniforms[36].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[36].uniformType);
	EXPECT_EQ(Type::Image2D, processor.uniforms[36].type);

	EXPECT_EQ("Image3D", processor.uniforms[37].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[37].uniformType);
	EXPECT_EQ(Type::Image3D, processor.uniforms[37].type);

	EXPECT_EQ("ImageCube", processor.uniforms[38].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[38].uniformType);
	EXPECT_EQ(Type::ImageCube, processor.uniforms[38].type);

	EXPECT_EQ("Image1DArray", processor.uniforms[39].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[39].uniformType);
	EXPECT_EQ(Type::Image1DArray, processor.uniforms[39].type);

	EXPECT_EQ("Image2DArray", processor.uniforms[40].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[40].uniformType);
	EXPECT_EQ(Type::Image2DArray, processor.uniforms[40].type);

	EXPECT_EQ("Image2DMS", processor.uniforms[41].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[41].uniformType);
	EXPECT_EQ(Type::Image2DMS, processor.uniforms[41].type);

	EXPECT_EQ("Image2DMSArray", processor.uniforms[42].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[42].uniformType);
	EXPECT_EQ(Type::Image2DMSArray, processor.uniforms[42].type);

	EXPECT_EQ("ImageBuffer", processor.uniforms[43].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[43].uniformType);
	EXPECT_EQ(Type::ImageBuffer, processor.uniforms[43].type);

	EXPECT_EQ("Image2DRect", processor.uniforms[44].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[44].uniformType);
	EXPECT_EQ(Type::Image2DRect, processor.uniforms[44].type);

	EXPECT_EQ("IImage1D", processor.uniforms[45].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[45].uniformType);
	EXPECT_EQ(Type::IImage1D, processor.uniforms[45].type);

	EXPECT_EQ("IImage2D", processor.uniforms[46].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[46].uniformType);
	EXPECT_EQ(Type::IImage2D, processor.uniforms[46].type);

	EXPECT_EQ("IImage3D", processor.uniforms[47].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[47].uniformType);
	EXPECT_EQ(Type::IImage3D, processor.uniforms[47].type);

	EXPECT_EQ("IImageCube", processor.uniforms[48].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[48].uniformType);
	EXPECT_EQ(Type::IImageCube, processor.uniforms[48].type);

	EXPECT_EQ("IImage1DArray", processor.uniforms[49].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[49].uniformType);
	EXPECT_EQ(Type::IImage1DArray, processor.uniforms[49].type);

	EXPECT_EQ("IImage2DArray", processor.uniforms[50].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[50].uniformType);
	EXPECT_EQ(Type::IImage2DArray, processor.uniforms[50].type);

	EXPECT_EQ("IImage2DMS", processor.uniforms[51].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[51].uniformType);
	EXPECT_EQ(Type::IImage2DMS, processor.uniforms[51].type);

	EXPECT_EQ("IImage2DMSArray", processor.uniforms[52].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[52].uniformType);
	EXPECT_EQ(Type::IImage2DMSArray, processor.uniforms[52].type);

	EXPECT_EQ("IImage2DRect", processor.uniforms[53].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[53].uniformType);
	EXPECT_EQ(Type::IImage2DRect, processor.uniforms[53].type);

	EXPECT_EQ("UImage1D", processor.uniforms[54].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[54].uniformType);
	EXPECT_EQ(Type::UImage1D, processor.uniforms[54].type);

	EXPECT_EQ("UImage2D", processor.uniforms[55].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[55].uniformType);
	EXPECT_EQ(Type::UImage2D, processor.uniforms[55].type);

	EXPECT_EQ("UImage3D", processor.uniforms[56].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[56].uniformType);
	EXPECT_EQ(Type::UImage3D, processor.uniforms[56].type);

	EXPECT_EQ("UImageCube", processor.uniforms[57].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[57].uniformType);
	EXPECT_EQ(Type::UImageCube, processor.uniforms[57].type);

	EXPECT_EQ("UImage1DArray", processor.uniforms[58].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[58].uniformType);
	EXPECT_EQ(Type::UImage1DArray, processor.uniforms[58].type);

	EXPECT_EQ("UImage2DArray", processor.uniforms[59].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[59].uniformType);
	EXPECT_EQ(Type::UImage2DArray, processor.uniforms[59].type);

	EXPECT_EQ("UImage2DMS", processor.uniforms[60].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[60].uniformType);
	EXPECT_EQ(Type::UImage2DMS, processor.uniforms[60].type);

	EXPECT_EQ("UImage2DMSArray", processor.uniforms[61].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[61].uniformType);
	EXPECT_EQ(Type::UImage2DMSArray, processor.uniforms[61].type);

	EXPECT_EQ("UImage2DRect", processor.uniforms[62].name);
	EXPECT_EQ(UniformType::Image, processor.uniforms[62].uniformType);
	EXPECT_EQ(Type::UImage2DRect, processor.uniforms[62].type);

	EXPECT_EQ("SubpassInput", processor.uniforms[63].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[63].uniformType);
	EXPECT_EQ(Type::SubpassInput, processor.uniforms[63].type);
	EXPECT_EQ(0U, processor.uniforms[63].inputAttachmentIndex);

	EXPECT_EQ("SubpassInputMS", processor.uniforms[64].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[64].uniformType);
	EXPECT_EQ(Type::SubpassInputMS, processor.uniforms[64].type);
	EXPECT_EQ(1U, processor.uniforms[64].inputAttachmentIndex);

	EXPECT_EQ("ISubpassInput", processor.uniforms[65].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[65].uniformType);
	EXPECT_EQ(Type::ISubpassInput, processor.uniforms[65].type);
	EXPECT_EQ(2U, processor.uniforms[65].inputAttachmentIndex);

	EXPECT_EQ("ISubpassInputMS", processor.uniforms[66].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[66].uniformType);
	EXPECT_EQ(Type::ISubpassInputMS, processor.uniforms[66].type);
	EXPECT_EQ(3U, processor.uniforms[66].inputAttachmentIndex);

	EXPECT_EQ("USubpassInput", processor.uniforms[67].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[67].uniformType);
	EXPECT_EQ(Type::USubpassInput, processor.uniforms[67].type);
	EXPECT_EQ(4U, processor.uniforms[67].inputAttachmentIndex);

	EXPECT_EQ("USubpassInputMS", processor.uniforms[68].name);
	EXPECT_EQ(UniformType::SubpassInput, processor.uniforms[68].uniformType);
	EXPECT_EQ(Type::USubpassInputMS, processor.uniforms[68].type);
	EXPECT_EQ(5U, processor.uniforms[68].inputAttachmentIndex);
}

TEST_F(SpirVProcessorTest, StructArrayReflection10)
{
	// Buffer blocks are treated differently starting SPIR-V 1.3.
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"StructArrayReflection.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), 0x10000));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV spirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor processor;
	EXPECT_TRUE(processor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, spirv, Stage::Fragment));
	EXPECT_TRUE(output.getMessages().empty());

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(4U, processor.structs.size());
	EXPECT_EQ("TestStruct", processor.structs[0].name);
	EXPECT_EQ(96U, processor.structs[0].size);
	ASSERT_EQ(3U, processor.structs[0].members.size());

	EXPECT_EQ("floatVar", processor.structs[0].members[0].name);
	EXPECT_EQ(0U, processor.structs[0].members[0].offset);
	EXPECT_EQ(sizeof(float), processor.structs[0].members[0].size);
	EXPECT_EQ(Type::Float, processor.structs[0].members[0].type);
	EXPECT_EQ(unknown, processor.structs[0].members[0].structIndex);
	EXPECT_TRUE(processor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(processor.structs[0].members[0].rowMajor);

	EXPECT_EQ("vec3Array", processor.structs[0].members[1].name);
	EXPECT_EQ(16U, processor.structs[0].members[1].offset);
	EXPECT_EQ(32U, processor.structs[0].members[1].size);
	EXPECT_EQ(Type::Vec3, processor.structs[0].members[1].type);
	EXPECT_EQ(unknown, processor.structs[0].members[1].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, 16U}}), processor.structs[0].members[1].arrayElements);
	EXPECT_FALSE(processor.structs[0].members[1].rowMajor);

	EXPECT_EQ("mat4x3Var", processor.structs[0].members[2].name);
	EXPECT_EQ(48U, processor.structs[0].members[2].offset);
	EXPECT_EQ(48U, processor.structs[0].members[2].size);
	EXPECT_EQ(Type::Mat4x3, processor.structs[0].members[2].type);
	EXPECT_EQ(unknown, processor.structs[0].members[2].structIndex);
	EXPECT_TRUE(processor.structs[0].members[2].arrayElements.empty());
	EXPECT_TRUE(processor.structs[0].members[2].rowMajor);

	EXPECT_EQ("TestBlock", processor.structs[1].name);
	EXPECT_EQ(512U, processor.structs[1].size);
	ASSERT_EQ(4U, processor.structs[1].members.size());

	EXPECT_EQ("vec2Array2D", processor.structs[1].members[0].name);
	EXPECT_EQ(0U, processor.structs[1].members[0].offset);
	EXPECT_EQ(96U, processor.structs[1].members[0].size);
	EXPECT_EQ(Type::Vec2, processor.structs[1].members[0].type);
	EXPECT_EQ(unknown, processor.structs[1].members[0].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 32U}, {2, 16U}}),
		processor.structs[1].members[0].arrayElements);
	EXPECT_FALSE(processor.structs[1].members[0].rowMajor);

	EXPECT_EQ("structMember", processor.structs[1].members[1].name);
	EXPECT_EQ(96U, processor.structs[1].members[1].offset);
	EXPECT_EQ(96U, processor.structs[1].members[1].size);
	EXPECT_EQ(Type::Struct, processor.structs[1].members[1].type);
	EXPECT_EQ(0U, processor.structs[1].members[1].structIndex);
	EXPECT_TRUE(processor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(processor.structs[1].members[1].rowMajor);

	EXPECT_EQ("structArray", processor.structs[1].members[2].name);
	EXPECT_EQ(192U, processor.structs[1].members[2].offset);
	EXPECT_EQ(288U, processor.structs[1].members[2].size);
	EXPECT_EQ(Type::Struct, processor.structs[1].members[2].type);
	EXPECT_EQ(0U, processor.structs[1].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 96U}}), processor.structs[1].members[2].arrayElements);
	EXPECT_FALSE(processor.structs[1].members[2].rowMajor);

	EXPECT_EQ("dvec3Var", processor.structs[1].members[3].name);
	EXPECT_EQ(480U, processor.structs[1].members[3].offset);
	EXPECT_EQ(3*sizeof(double), processor.structs[1].members[3].size);
	EXPECT_EQ(Type::DVec3, processor.structs[1].members[3].type);
	EXPECT_EQ(unknown, processor.structs[1].members[3].structIndex);
	EXPECT_TRUE(processor.structs[1].members[3].arrayElements.empty());
	EXPECT_FALSE(processor.structs[1].members[3].rowMajor);

	EXPECT_EQ("TestBufferStruct", processor.structs[2].name);
	EXPECT_EQ(112U, processor.structs[2].size);
	ASSERT_EQ(3U, processor.structs[2].members.size());

	EXPECT_EQ("floatVar", processor.structs[2].members[0].name);
	EXPECT_EQ(0U, processor.structs[2].members[0].offset);
	EXPECT_EQ(sizeof(float), processor.structs[2].members[0].size);
	EXPECT_EQ(Type::Float, processor.structs[2].members[0].type);
	EXPECT_EQ(unknown, processor.structs[2].members[0].structIndex);
	EXPECT_TRUE(processor.structs[2].members[0].arrayElements.empty());
	EXPECT_FALSE(processor.structs[2].members[0].rowMajor);

	EXPECT_EQ("vec3Array", processor.structs[2].members[1].name);
	EXPECT_EQ(16U, processor.structs[2].members[1].offset);
	EXPECT_EQ(32U, processor.structs[2].members[1].size);
	EXPECT_EQ(Type::Vec3, processor.structs[2].members[1].type);
	EXPECT_EQ(unknown, processor.structs[2].members[1].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, 16U}}), processor.structs[2].members[1].arrayElements);
	EXPECT_FALSE(processor.structs[2].members[1].rowMajor);

	EXPECT_EQ("mat4x3Var", processor.structs[2].members[2].name);
	EXPECT_EQ(48U, processor.structs[2].members[2].offset);
	EXPECT_EQ(64U, processor.structs[2].members[2].size);
	EXPECT_EQ(Type::Mat4x3, processor.structs[2].members[2].type);
	EXPECT_EQ(unknown, processor.structs[2].members[2].structIndex);
	EXPECT_TRUE(processor.structs[2].members[2].arrayElements.empty());
	EXPECT_FALSE(processor.structs[2].members[2].rowMajor);

	EXPECT_EQ("TestBuffer", processor.structs[3].name);
	EXPECT_EQ(536U, processor.structs[3].size);
	ASSERT_EQ(5U, processor.structs[3].members.size());

	EXPECT_EQ("vec2Array2D", processor.structs[3].members[0].name);
	EXPECT_EQ(0U, processor.structs[3].members[0].offset);
	EXPECT_EQ(48U, processor.structs[3].members[0].size);
	EXPECT_EQ(Type::Vec2, processor.structs[3].members[0].type);
	EXPECT_EQ(unknown, processor.structs[3].members[0].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 16U}, {2, 8U}}),
		processor.structs[3].members[0].arrayElements);
	EXPECT_FALSE(processor.structs[3].members[0].rowMajor);

	EXPECT_EQ("structMember", processor.structs[3].members[1].name);
	EXPECT_EQ(48U, processor.structs[3].members[1].offset);
	EXPECT_EQ(112U, processor.structs[3].members[1].size);
	EXPECT_EQ(Type::Struct, processor.structs[3].members[1].type);
	EXPECT_EQ(2U, processor.structs[3].members[1].structIndex);
	EXPECT_TRUE(processor.structs[3].members[1].arrayElements.empty());
	EXPECT_FALSE(processor.structs[3].members[1].rowMajor);

	EXPECT_EQ("structArray", processor.structs[3].members[2].name);
	EXPECT_EQ(160U, processor.structs[3].members[2].offset);
	EXPECT_EQ(336U, processor.structs[3].members[2].size);
	EXPECT_EQ(Type::Struct, processor.structs[3].members[2].type);
	EXPECT_EQ(2U, processor.structs[3].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 112U}}), processor.structs[3].members[2].arrayElements);
	EXPECT_FALSE(processor.structs[3].members[2].rowMajor);

	EXPECT_EQ("dvec3Var", processor.structs[3].members[3].name);
	EXPECT_EQ(512U, processor.structs[3].members[3].offset);
	EXPECT_EQ(3*sizeof(double), processor.structs[3].members[3].size);
	EXPECT_EQ(Type::DVec3, processor.structs[3].members[3].type);
	EXPECT_EQ(unknown, processor.structs[3].members[3].structIndex);
	EXPECT_TRUE(processor.structs[3].members[3].arrayElements.empty());
	EXPECT_FALSE(processor.structs[3].members[3].rowMajor);

	EXPECT_EQ("dynamicArray", processor.structs[3].members[4].name);
	EXPECT_EQ(536U, processor.structs[3].members[4].offset);
	EXPECT_EQ(unknown, processor.structs[3].members[4].size);
	EXPECT_EQ(Type::Float, processor.structs[3].members[4].type);
	EXPECT_EQ(unknown, processor.structs[3].members[4].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{unknown, (std::uint32_t)(3*sizeof(float))},
		{3, (std::uint32_t)sizeof(float)}}), processor.structs[3].members[4].arrayElements);
	EXPECT_FALSE(processor.structs[3].members[4].rowMajor);

	ASSERT_EQ(3U, processor.uniforms.size());
	EXPECT_EQ("TestBlock", processor.uniforms[0].name);
	EXPECT_EQ(UniformType::Block, processor.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, processor.uniforms[0].type);
	EXPECT_EQ(1U, processor.uniforms[0].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, unknown}}), processor.uniforms[0].arrayElements);
	EXPECT_EQ(1U, processor.uniforms[0].descriptorSet);
	EXPECT_EQ(0U, processor.uniforms[0].binding);
	EXPECT_EQ(unknown, processor.uniforms[0].samplerIndex);

	EXPECT_EQ("TestBuffer", processor.uniforms[1].name);
	EXPECT_EQ(UniformType::BlockBuffer, processor.uniforms[1].uniformType);
	EXPECT_EQ(Type::Struct, processor.uniforms[1].type);
	EXPECT_EQ(3U, processor.uniforms[1].structIndex);
	EXPECT_TRUE(processor.uniforms[1].arrayElements.empty());
	EXPECT_EQ(2U, processor.uniforms[1].descriptorSet);
	EXPECT_EQ(1U, processor.uniforms[1].binding);
	EXPECT_EQ(unknown, processor.uniforms[1].samplerIndex);

	EXPECT_EQ("samplerArray", processor.uniforms[2].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[2].uniformType);
	EXPECT_EQ(Type::Sampler2D, processor.uniforms[2].type);
	EXPECT_EQ(unknown, processor.uniforms[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{4, unknown}}), processor.uniforms[2].arrayElements);
	EXPECT_EQ(3U, processor.uniforms[2].descriptorSet);
	EXPECT_EQ(2U, processor.uniforms[2].binding);
	EXPECT_EQ(unknown, processor.uniforms[2].samplerIndex);
}

TEST_F(SpirVProcessorTest, StructArrayReflectionLatest)
{
	// Buffer blocks are treated differently starting SPIR-V 1.3.
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"StructArrayReflection.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV spirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor processor;
	EXPECT_TRUE(processor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, spirv, Stage::Fragment));
	EXPECT_TRUE(output.getMessages().empty());

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(4U, processor.structs.size());
	EXPECT_EQ("TestStruct", processor.structs[0].name);
	EXPECT_EQ(96U, processor.structs[0].size);
	ASSERT_EQ(3U, processor.structs[0].members.size());

	EXPECT_EQ("floatVar", processor.structs[0].members[0].name);
	EXPECT_EQ(0U, processor.structs[0].members[0].offset);
	EXPECT_EQ(sizeof(float), processor.structs[0].members[0].size);
	EXPECT_EQ(Type::Float, processor.structs[0].members[0].type);
	EXPECT_EQ(unknown, processor.structs[0].members[0].structIndex);
	EXPECT_TRUE(processor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(processor.structs[0].members[0].rowMajor);

	EXPECT_EQ("vec3Array", processor.structs[0].members[1].name);
	EXPECT_EQ(16U, processor.structs[0].members[1].offset);
	EXPECT_EQ(32U, processor.structs[0].members[1].size);
	EXPECT_EQ(Type::Vec3, processor.structs[0].members[1].type);
	EXPECT_EQ(unknown, processor.structs[0].members[1].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, 16U}}), processor.structs[0].members[1].arrayElements);
	EXPECT_FALSE(processor.structs[0].members[1].rowMajor);

	EXPECT_EQ("mat4x3Var", processor.structs[0].members[2].name);
	EXPECT_EQ(48U, processor.structs[0].members[2].offset);
	EXPECT_EQ(48U, processor.structs[0].members[2].size);
	EXPECT_EQ(Type::Mat4x3, processor.structs[0].members[2].type);
	EXPECT_EQ(unknown, processor.structs[0].members[2].structIndex);
	EXPECT_TRUE(processor.structs[0].members[2].arrayElements.empty());
	EXPECT_TRUE(processor.structs[0].members[2].rowMajor);

	EXPECT_EQ("TestBlock", processor.structs[1].name);
	EXPECT_EQ(512U, processor.structs[1].size);
	ASSERT_EQ(4U, processor.structs[1].members.size());

	EXPECT_EQ("vec2Array2D", processor.structs[1].members[0].name);
	EXPECT_EQ(0U, processor.structs[1].members[0].offset);
	EXPECT_EQ(96U, processor.structs[1].members[0].size);
	EXPECT_EQ(Type::Vec2, processor.structs[1].members[0].type);
	EXPECT_EQ(unknown, processor.structs[1].members[0].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 32U}, {2, 16U}}),
		processor.structs[1].members[0].arrayElements);
	EXPECT_FALSE(processor.structs[1].members[0].rowMajor);

	EXPECT_EQ("structMember", processor.structs[1].members[1].name);
	EXPECT_EQ(96U, processor.structs[1].members[1].offset);
	EXPECT_EQ(96U, processor.structs[1].members[1].size);
	EXPECT_EQ(Type::Struct, processor.structs[1].members[1].type);
	EXPECT_EQ(0U, processor.structs[1].members[1].structIndex);
	EXPECT_TRUE(processor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(processor.structs[1].members[1].rowMajor);

	EXPECT_EQ("structArray", processor.structs[1].members[2].name);
	EXPECT_EQ(192U, processor.structs[1].members[2].offset);
	EXPECT_EQ(288U, processor.structs[1].members[2].size);
	EXPECT_EQ(Type::Struct, processor.structs[1].members[2].type);
	EXPECT_EQ(0U, processor.structs[1].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 96U}}), processor.structs[1].members[2].arrayElements);
	EXPECT_FALSE(processor.structs[1].members[2].rowMajor);

	EXPECT_EQ("dvec3Var", processor.structs[1].members[3].name);
	EXPECT_EQ(480U, processor.structs[1].members[3].offset);
	EXPECT_EQ(3*sizeof(double), processor.structs[1].members[3].size);
	EXPECT_EQ(Type::DVec3, processor.structs[1].members[3].type);
	EXPECT_EQ(unknown, processor.structs[1].members[3].structIndex);
	EXPECT_TRUE(processor.structs[1].members[3].arrayElements.empty());
	EXPECT_FALSE(processor.structs[1].members[3].rowMajor);

	EXPECT_EQ("TestBufferStruct", processor.structs[2].name);
	EXPECT_EQ(112U, processor.structs[2].size);
	ASSERT_EQ(3U, processor.structs[2].members.size());

	EXPECT_EQ("floatVar", processor.structs[2].members[0].name);
	EXPECT_EQ(0U, processor.structs[2].members[0].offset);
	EXPECT_EQ(sizeof(float), processor.structs[2].members[0].size);
	EXPECT_EQ(Type::Float, processor.structs[2].members[0].type);
	EXPECT_EQ(unknown, processor.structs[2].members[0].structIndex);
	EXPECT_TRUE(processor.structs[2].members[0].arrayElements.empty());
	EXPECT_FALSE(processor.structs[2].members[0].rowMajor);

	EXPECT_EQ("vec3Array", processor.structs[2].members[1].name);
	EXPECT_EQ(16U, processor.structs[2].members[1].offset);
	EXPECT_EQ(32U, processor.structs[2].members[1].size);
	EXPECT_EQ(Type::Vec3, processor.structs[2].members[1].type);
	EXPECT_EQ(unknown, processor.structs[2].members[1].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, 16U}}), processor.structs[2].members[1].arrayElements);
	EXPECT_FALSE(processor.structs[2].members[1].rowMajor);

	EXPECT_EQ("mat4x3Var", processor.structs[2].members[2].name);
	EXPECT_EQ(48U, processor.structs[2].members[2].offset);
	EXPECT_EQ(64U, processor.structs[2].members[2].size);
	EXPECT_EQ(Type::Mat4x3, processor.structs[2].members[2].type);
	EXPECT_EQ(unknown, processor.structs[2].members[2].structIndex);
	EXPECT_TRUE(processor.structs[2].members[2].arrayElements.empty());
	EXPECT_FALSE(processor.structs[2].members[2].rowMajor);

	EXPECT_EQ("TestBuffer", processor.structs[3].name);
	EXPECT_EQ(536U, processor.structs[3].size);
	ASSERT_EQ(5U, processor.structs[3].members.size());

	EXPECT_EQ("vec2Array2D", processor.structs[3].members[0].name);
	EXPECT_EQ(0U, processor.structs[3].members[0].offset);
	EXPECT_EQ(48U, processor.structs[3].members[0].size);
	EXPECT_EQ(Type::Vec2, processor.structs[3].members[0].type);
	EXPECT_EQ(unknown, processor.structs[3].members[0].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 16U}, {2, 8U}}),
		processor.structs[3].members[0].arrayElements);
	EXPECT_FALSE(processor.structs[3].members[0].rowMajor);

	EXPECT_EQ("structMember", processor.structs[3].members[1].name);
	EXPECT_EQ(48U, processor.structs[3].members[1].offset);
	EXPECT_EQ(112U, processor.structs[3].members[1].size);
	EXPECT_EQ(Type::Struct, processor.structs[3].members[1].type);
	EXPECT_EQ(2U, processor.structs[3].members[1].structIndex);
	EXPECT_TRUE(processor.structs[3].members[1].arrayElements.empty());
	EXPECT_FALSE(processor.structs[3].members[1].rowMajor);

	EXPECT_EQ("structArray", processor.structs[3].members[2].name);
	EXPECT_EQ(160U, processor.structs[3].members[2].offset);
	EXPECT_EQ(336U, processor.structs[3].members[2].size);
	EXPECT_EQ(Type::Struct, processor.structs[3].members[2].type);
	EXPECT_EQ(2U, processor.structs[3].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, 112U}}), processor.structs[3].members[2].arrayElements);
	EXPECT_FALSE(processor.structs[3].members[2].rowMajor);

	EXPECT_EQ("dvec3Var", processor.structs[3].members[3].name);
	EXPECT_EQ(512U, processor.structs[3].members[3].offset);
	EXPECT_EQ(3*sizeof(double), processor.structs[3].members[3].size);
	EXPECT_EQ(Type::DVec3, processor.structs[3].members[3].type);
	EXPECT_EQ(unknown, processor.structs[3].members[3].structIndex);
	EXPECT_TRUE(processor.structs[3].members[3].arrayElements.empty());
	EXPECT_FALSE(processor.structs[3].members[3].rowMajor);

	EXPECT_EQ("dynamicArray", processor.structs[3].members[4].name);
	EXPECT_EQ(536U, processor.structs[3].members[4].offset);
	EXPECT_EQ(unknown, processor.structs[3].members[4].size);
	EXPECT_EQ(Type::Float, processor.structs[3].members[4].type);
	EXPECT_EQ(unknown, processor.structs[3].members[4].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{unknown, (std::uint32_t)(3*sizeof(float))},
		{3, (std::uint32_t)sizeof(float)}}), processor.structs[3].members[4].arrayElements);
	EXPECT_FALSE(processor.structs[3].members[4].rowMajor);

	ASSERT_EQ(3U, processor.uniforms.size());
	EXPECT_EQ("TestBlock", processor.uniforms[0].name);
	EXPECT_EQ(UniformType::Block, processor.uniforms[0].uniformType);
	EXPECT_EQ(Type::Struct, processor.uniforms[0].type);
	EXPECT_EQ(1U, processor.uniforms[0].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, unknown}}), processor.uniforms[0].arrayElements);
	EXPECT_EQ(1U, processor.uniforms[0].descriptorSet);
	EXPECT_EQ(0U, processor.uniforms[0].binding);
	EXPECT_EQ(unknown, processor.uniforms[0].samplerIndex);

	EXPECT_EQ("samplerArray", processor.uniforms[1].name);
	EXPECT_EQ(UniformType::SampledImage, processor.uniforms[1].uniformType);
	EXPECT_EQ(Type::Sampler2D, processor.uniforms[1].type);
	EXPECT_EQ(unknown, processor.uniforms[1].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{4, unknown}}), processor.uniforms[1].arrayElements);
	EXPECT_EQ(3U, processor.uniforms[1].descriptorSet);
	EXPECT_EQ(2U, processor.uniforms[1].binding);
	EXPECT_EQ(unknown, processor.uniforms[1].samplerIndex);

	EXPECT_EQ("TestBuffer", processor.uniforms[2].name);
	EXPECT_EQ(UniformType::BlockBuffer, processor.uniforms[2].uniformType);
	EXPECT_EQ(Type::Struct, processor.uniforms[2].type);
	EXPECT_EQ(3U, processor.uniforms[2].structIndex);
	EXPECT_TRUE(processor.uniforms[2].arrayElements.empty());
	EXPECT_EQ(2U, processor.uniforms[2].descriptorSet);
	EXPECT_EQ(1U, processor.uniforms[2].binding);
	EXPECT_EQ(unknown, processor.uniforms[2].samplerIndex);
}

TEST_F(SpirVProcessorTest, InputsOutputs)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"InputsOutputs.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_TRUE(fragmentProcessor.linkInputs(output, vertexProcessor));

	EXPECT_TRUE(output.getMessages().empty());

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(23U, vertexProcessor.outputs.size());

	EXPECT_EQ("vec2Val", vertexProcessor.outputs[0].name);
	EXPECT_EQ(Type::Vec2, vertexProcessor.outputs[0].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[0].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[0].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].component);

	EXPECT_EQ("vec3Val", vertexProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.outputs[1].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[1].structIndex);
	EXPECT_EQ((std::vector<std::uint32_t>{2U}), vertexProcessor.outputs[1].arrayElements);
	EXPECT_TRUE(vertexProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[1].patch);
	EXPECT_EQ(1U, vertexProcessor.outputs[1].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[1].component);

	EXPECT_EQ("vec4Val", vertexProcessor.outputs[2].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.outputs[2].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[2].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[2].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[2].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[2].patch);
	EXPECT_EQ(3U, vertexProcessor.outputs[2].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[2].component);

	EXPECT_EQ("doubleVal", vertexProcessor.outputs[3].name);
	EXPECT_EQ(Type::Double, vertexProcessor.outputs[3].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[3].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[3].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[3].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[3].patch);
	EXPECT_EQ(4U, vertexProcessor.outputs[3].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[3].component);

	EXPECT_EQ("dvec3Val", vertexProcessor.outputs[4].name);
	EXPECT_EQ(Type::DVec3, vertexProcessor.outputs[4].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[4].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[4].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[4].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[4].patch);
	EXPECT_EQ(5U, vertexProcessor.outputs[4].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[4].component);

	EXPECT_EQ("dvec4Val", vertexProcessor.outputs[5].name);
	EXPECT_EQ(Type::DVec4, vertexProcessor.outputs[5].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[5].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[5].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[5].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[5].patch);
	EXPECT_EQ(7U, vertexProcessor.outputs[5].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[5].component);

	EXPECT_EQ("intVal", vertexProcessor.outputs[6].name);
	EXPECT_EQ(Type::Int, vertexProcessor.outputs[6].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[6].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[6].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[6].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[6].patch);
	EXPECT_EQ(9U, vertexProcessor.outputs[6].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[6].component);

	EXPECT_EQ("ivec2Val", vertexProcessor.outputs[7].name);
	EXPECT_EQ(Type::IVec2, vertexProcessor.outputs[7].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[7].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[7].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[7].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[7].patch);
	EXPECT_EQ(10U, vertexProcessor.outputs[7].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[7].component);

	EXPECT_EQ("ivec4Val", vertexProcessor.outputs[8].name);
	EXPECT_EQ(Type::IVec4, vertexProcessor.outputs[8].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[8].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[8].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[8].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[8].patch);
	EXPECT_EQ(11U, vertexProcessor.outputs[8].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[8].component);

	EXPECT_EQ("uintVal", vertexProcessor.outputs[9].name);
	EXPECT_EQ(Type::UInt, vertexProcessor.outputs[9].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[9].structIndex);
	EXPECT_EQ((std::vector<std::uint32_t>{3U}), vertexProcessor.outputs[9].arrayElements);
	EXPECT_TRUE(vertexProcessor.outputs[9].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[9].patch);
	EXPECT_EQ(12U, vertexProcessor.outputs[9].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[9].component);

	EXPECT_EQ("uvec2Val", vertexProcessor.outputs[10].name);
	EXPECT_EQ(Type::UVec2, vertexProcessor.outputs[10].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[10].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[10].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[10].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[10].patch);
	EXPECT_EQ(15U, vertexProcessor.outputs[10].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[10].component);

	EXPECT_EQ("uvec3Val", vertexProcessor.outputs[11].name);
	EXPECT_EQ(Type::UVec3, vertexProcessor.outputs[11].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[11].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[11].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[11].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[11].patch);
	EXPECT_EQ(16U, vertexProcessor.outputs[11].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[11].component);

	EXPECT_EQ("mat2Val", vertexProcessor.outputs[12].name);
	EXPECT_EQ(Type::Mat2, vertexProcessor.outputs[12].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[12].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[12].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[12].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[12].patch);
	EXPECT_EQ(17U, vertexProcessor.outputs[12].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[12].component);

	EXPECT_EQ("mat4Val", vertexProcessor.outputs[13].name);
	EXPECT_EQ(Type::Mat4, vertexProcessor.outputs[13].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[13].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[13].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[13].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[13].patch);
	EXPECT_EQ(19U, vertexProcessor.outputs[13].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[13].component);

	EXPECT_EQ("mat2x3Val", vertexProcessor.outputs[14].name);
	EXPECT_EQ(Type::Mat2x3, vertexProcessor.outputs[14].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[14].structIndex);
	EXPECT_EQ((std::vector<std::uint32_t>{4U}), vertexProcessor.outputs[14].arrayElements);
	EXPECT_TRUE(vertexProcessor.outputs[14].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[14].patch);
	EXPECT_EQ(23U, vertexProcessor.outputs[14].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[14].component);

	EXPECT_EQ("mat3x2Val", vertexProcessor.outputs[15].name);
	EXPECT_EQ(Type::Mat3x2, vertexProcessor.outputs[15].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[15].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[15].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[15].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[15].patch);
	EXPECT_EQ(31U, vertexProcessor.outputs[15].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[15].component);

	EXPECT_EQ("mat4x3Val", vertexProcessor.outputs[16].name);
	EXPECT_EQ(Type::Mat4x3, vertexProcessor.outputs[16].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[16].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[16].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[16].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[16].patch);
	EXPECT_EQ(34U, vertexProcessor.outputs[16].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[16].component);

	EXPECT_EQ("dmat3Val", vertexProcessor.outputs[17].name);
	EXPECT_EQ(Type::DMat3, vertexProcessor.outputs[17].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[17].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[17].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[17].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[17].patch);
	EXPECT_EQ(38U, vertexProcessor.outputs[17].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[17].component);

	EXPECT_EQ("dmat4Val", vertexProcessor.outputs[18].name);
	EXPECT_EQ(Type::DMat4, vertexProcessor.outputs[18].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[18].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[18].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[18].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[18].patch);
	EXPECT_EQ(44U, vertexProcessor.outputs[18].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[18].component);

	EXPECT_EQ("dmat2x4Val", vertexProcessor.outputs[19].name);
	EXPECT_EQ(Type::DMat2x4, vertexProcessor.outputs[19].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[19].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[19].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[19].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[19].patch);
	EXPECT_EQ(52U, vertexProcessor.outputs[19].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[19].component);

	EXPECT_EQ("dmat3x4Val", vertexProcessor.outputs[20].name);
	EXPECT_EQ(Type::DMat3x4, vertexProcessor.outputs[20].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[20].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[20].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[20].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[20].patch);
	EXPECT_EQ(56U, vertexProcessor.outputs[20].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[20].component);

	EXPECT_EQ("dmat4x2Val", vertexProcessor.outputs[21].name);
	EXPECT_EQ(Type::DMat4x2, vertexProcessor.outputs[21].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[21].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[21].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[21].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[21].patch);
	EXPECT_EQ(62U, vertexProcessor.outputs[21].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[21].component);

	EXPECT_EQ("", vertexProcessor.outputs[22].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.outputs[22].type);
	EXPECT_EQ(1U, vertexProcessor.outputs[22].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[22].arrayElements.empty());
	ASSERT_EQ(13U, vertexProcessor.outputs[22].memberLocations.size());
	EXPECT_FALSE(vertexProcessor.outputs[22].patch);
	EXPECT_EQ(unknown, vertexProcessor.outputs[22].location);
	EXPECT_EQ(unknown, vertexProcessor.outputs[22].component);

	ASSERT_EQ(2U, vertexProcessor.structs.size());
	EXPECT_EQ("VertFragBlock", vertexProcessor.structs[1].name);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].size);

	ASSERT_EQ(13U, vertexProcessor.structs[1].members.size());
	EXPECT_EQ("floatVal", vertexProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::Float, vertexProcessor.structs[1].members[0].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[0].rowMajor);
	EXPECT_EQ(std::make_pair(66U, 0U), vertexProcessor.outputs[22].memberLocations[0]);

	EXPECT_EQ("dvec2Val", vertexProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::DVec2, vertexProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[1].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[1].rowMajor);
	EXPECT_EQ(std::make_pair(67U, 0U), vertexProcessor.outputs[22].memberLocations[1]);

	EXPECT_EQ("ivec3Val", vertexProcessor.structs[1].members[2].name);
	EXPECT_EQ(Type::IVec3, vertexProcessor.structs[1].members[2].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[2].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[2].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[2].rowMajor);
	EXPECT_EQ(std::make_pair(68U, 0U), vertexProcessor.outputs[22].memberLocations[2]);

	EXPECT_EQ("uvec4Val", vertexProcessor.structs[1].members[3].name);
	EXPECT_EQ(Type::UVec4, vertexProcessor.structs[1].members[3].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[3].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[3].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[3].rowMajor);
	EXPECT_EQ(std::make_pair(69U, 0U), vertexProcessor.outputs[22].memberLocations[3]);

	EXPECT_EQ("mat3Val", vertexProcessor.structs[1].members[4].name);
	EXPECT_EQ(Type::Mat3, vertexProcessor.structs[1].members[4].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[4].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[4].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[4].rowMajor);
	EXPECT_EQ(std::make_pair(70U, 0U), vertexProcessor.outputs[22].memberLocations[4]);

	EXPECT_EQ("mat2x4Val", vertexProcessor.structs[1].members[5].name);
	EXPECT_EQ(Type::Mat2x4, vertexProcessor.structs[1].members[5].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[5].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[5].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[5].rowMajor);
	EXPECT_EQ(std::make_pair(73U, 0U), vertexProcessor.outputs[22].memberLocations[5]);

	EXPECT_EQ("mat3x4Val", vertexProcessor.structs[1].members[6].name);
	EXPECT_EQ(Type::Mat3x4, vertexProcessor.structs[1].members[6].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[6].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[6].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[6].rowMajor);
	EXPECT_EQ(std::make_pair(75U, 0U), vertexProcessor.outputs[22].memberLocations[6]);

	EXPECT_EQ("mat4x2Val", vertexProcessor.structs[1].members[7].name);
	EXPECT_EQ(Type::Mat4x2, vertexProcessor.structs[1].members[7].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[7].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[7].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[7].rowMajor);
	EXPECT_EQ(std::make_pair(78U, 0U), vertexProcessor.outputs[22].memberLocations[7]);

	EXPECT_EQ("dmat2Val", vertexProcessor.structs[1].members[8].name);
	EXPECT_EQ(Type::DMat2, vertexProcessor.structs[1].members[8].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[8].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[8].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[8].rowMajor);
	EXPECT_EQ(std::make_pair(82U, 0U), vertexProcessor.outputs[22].memberLocations[8]);

	EXPECT_EQ("dmat2x3Val", vertexProcessor.structs[1].members[9].name);
	EXPECT_EQ(Type::DMat2x3, vertexProcessor.structs[1].members[9].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[9].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[9].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[9].rowMajor);
	EXPECT_EQ(std::make_pair(84U, 0U), vertexProcessor.outputs[22].memberLocations[9]);

	EXPECT_EQ("dmat3x2Val", vertexProcessor.structs[1].members[10].name);
	EXPECT_EQ(Type::DMat3x2, vertexProcessor.structs[1].members[10].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[10].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{5, unknown}, {2, unknown}}),
		vertexProcessor.structs[1].members[10].arrayElements);
	EXPECT_FALSE(vertexProcessor.structs[1].members[10].rowMajor);
	EXPECT_EQ(std::make_pair(88U, 0U), vertexProcessor.outputs[22].memberLocations[10]);

	EXPECT_EQ("dmat4x3Val", vertexProcessor.structs[1].members[11].name);
	EXPECT_EQ(Type::DMat4x3, vertexProcessor.structs[1].members[11].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[11].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[11].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[11].rowMajor);
	EXPECT_EQ(std::make_pair(118U, 0U), vertexProcessor.outputs[22].memberLocations[11]);

	EXPECT_EQ("paddingVal", vertexProcessor.structs[1].members[12].name);
	EXPECT_EQ(Type::Float, vertexProcessor.structs[1].members[12].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[12].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[12].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[12].rowMajor);
	EXPECT_EQ(std::make_pair(126U, 0U), vertexProcessor.outputs[22].memberLocations[12]);

	ASSERT_EQ(vertexProcessor.outputs.size(), fragmentProcessor.inputs.size());
	for (std::size_t i = 0; i < 22; ++i)
	{
		EXPECT_EQ(vertexProcessor.outputs[i].name, fragmentProcessor.inputs[i].name);
		EXPECT_EQ(vertexProcessor.outputs[i].type, fragmentProcessor.inputs[i].type);
		EXPECT_EQ(unknown, fragmentProcessor.inputs[i].structIndex);
		EXPECT_EQ(vertexProcessor.outputs[i].arrayElements,
			fragmentProcessor.inputs[i].arrayElements);
		EXPECT_TRUE(fragmentProcessor.inputs[i].memberLocations.empty());
		EXPECT_FALSE(fragmentProcessor.inputs[i].patch);
		EXPECT_EQ(vertexProcessor.outputs[i].location, fragmentProcessor.inputs[i].location);
		EXPECT_EQ(vertexProcessor.outputs[i].component, fragmentProcessor.inputs[i].component);
	}

	ASSERT_EQ(1U, fragmentProcessor.structs.size());
	EXPECT_EQ("VertFragBlock", fragmentProcessor.structs[0].name);
	ASSERT_EQ(vertexProcessor.outputs[22].memberLocations.size(),
		fragmentProcessor.inputs[22].memberLocations.size());
	ASSERT_EQ(vertexProcessor.structs[1].members.size(),
		fragmentProcessor.structs[0].members.size());
	for (std::size_t i = 0; i < vertexProcessor.structs[1].members.size(); ++i)
	{
		EXPECT_EQ(vertexProcessor.structs[1].members[i].name,
			fragmentProcessor.structs[0].members[i].name);
		EXPECT_EQ(vertexProcessor.structs[1].members[i].type,
			fragmentProcessor.structs[0].members[i].type);
		EXPECT_EQ(unknown, vertexProcessor.structs[1].members[12].structIndex);
		EXPECT_EQ(vertexProcessor.structs[1].members[i].arrayElements,
			fragmentProcessor.structs[0].members[i].arrayElements);
		EXPECT_FALSE(fragmentProcessor.structs[0].members[i].rowMajor);
		EXPECT_EQ(vertexProcessor.outputs[22].memberLocations[i],
			fragmentProcessor.inputs[22].memberLocations[i]);
	}
}

TEST_F(SpirVProcessorTest, ExplicitInputsOutputs)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"ExplicitInputsOutputs.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_TRUE(fragmentProcessor.linkInputs(output, vertexProcessor));

	EXPECT_TRUE(output.getMessages().empty());

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(5U, vertexProcessor.outputs.size());

	EXPECT_EQ("floatVal", vertexProcessor.outputs[0].name);
	EXPECT_EQ(Type::Float, vertexProcessor.outputs[0].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[0].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[0].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].component);

	EXPECT_EQ("vec2Val", vertexProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec2, vertexProcessor.outputs[1].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[1].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[1].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[1].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[1].location);
	EXPECT_EQ(2U, vertexProcessor.outputs[1].component);

	EXPECT_EQ("ivec3Val", vertexProcessor.outputs[2].name);
	EXPECT_EQ(Type::IVec3, vertexProcessor.outputs[2].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[2].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[2].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[2].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[2].patch);
	EXPECT_EQ(1U, vertexProcessor.outputs[2].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[2].component);

	EXPECT_EQ("intVal", vertexProcessor.outputs[3].name);
	EXPECT_EQ(Type::Int, vertexProcessor.outputs[3].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[3].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[3].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[3].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[3].patch);
	EXPECT_EQ(1U, vertexProcessor.outputs[3].location);
	EXPECT_EQ(3U, vertexProcessor.outputs[3].component);

	EXPECT_EQ("", vertexProcessor.outputs[4].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.outputs[4].type);
	EXPECT_EQ(1U, vertexProcessor.outputs[4].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[4].arrayElements.empty());
	ASSERT_EQ(2U, vertexProcessor.outputs[4].memberLocations.size());
	EXPECT_FALSE(vertexProcessor.outputs[4].patch);
	EXPECT_EQ(2U, vertexProcessor.outputs[4].location);
	EXPECT_EQ(unknown, vertexProcessor.outputs[4].component);

	ASSERT_EQ(2U, vertexProcessor.structs.size());
	EXPECT_EQ("OutBlock", vertexProcessor.structs[1].name);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].size);

	ASSERT_EQ(2U, vertexProcessor.structs[1].members.size());
	EXPECT_EQ("dvec3Val", vertexProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::DVec3, vertexProcessor.structs[1].members[0].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[0].rowMajor);
	EXPECT_EQ(std::make_pair(2U, 0U), vertexProcessor.outputs[4].memberLocations[0]);

	EXPECT_EQ("mat4Val", vertexProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Mat4, vertexProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[1].rowMajor);
	EXPECT_EQ(std::make_pair(4U, 0U), vertexProcessor.outputs[4].memberLocations[1]);

	ASSERT_EQ(5U, fragmentProcessor.inputs.size());

	EXPECT_EQ("", fragmentProcessor.inputs[0].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.inputs[0].type);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[0].arrayElements.empty());
	ASSERT_EQ(2U, fragmentProcessor.inputs[0].memberLocations.size());
	EXPECT_FALSE(fragmentProcessor.inputs[0].patch);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[0].location);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[0].component);

	EXPECT_EQ("intVal", fragmentProcessor.inputs[1].name);
	EXPECT_EQ(Type::Int, fragmentProcessor.inputs[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[1].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[1].patch);
	EXPECT_EQ(1U, fragmentProcessor.inputs[1].location);
	EXPECT_EQ(3U, fragmentProcessor.inputs[1].component);

	EXPECT_EQ("ivec3Val", fragmentProcessor.inputs[2].name);
	EXPECT_EQ(Type::IVec3, fragmentProcessor.inputs[2].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[2].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[2].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[2].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[2].patch);
	EXPECT_EQ(1U, fragmentProcessor.inputs[2].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[2].component);

	EXPECT_EQ("vec2Val", fragmentProcessor.inputs[3].name);
	EXPECT_EQ(Type::Vec2, fragmentProcessor.inputs[3].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[3].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[3].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[3].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[3].patch);
	EXPECT_EQ(0U, fragmentProcessor.inputs[3].location);
	EXPECT_EQ(2U, fragmentProcessor.inputs[3].component);

	EXPECT_EQ("floatVal", fragmentProcessor.inputs[4].name);
	EXPECT_EQ(Type::Float, fragmentProcessor.inputs[4].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[4].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[4].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[4].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[4].patch);
	EXPECT_EQ(0U, fragmentProcessor.inputs[4].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[4].component);

	ASSERT_EQ(1U, fragmentProcessor.structs.size());
	EXPECT_EQ("InBlock", fragmentProcessor.structs[0].name);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].size);

	ASSERT_EQ(2U, fragmentProcessor.structs[0].members.size());
	EXPECT_EQ("mat4Val", fragmentProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Mat4, fragmentProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[0].rowMajor);
	EXPECT_EQ(std::make_pair(4U, 0U), fragmentProcessor.inputs[0].memberLocations[0]);

	EXPECT_EQ("dvec3Val", fragmentProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::DVec3, fragmentProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[1].rowMajor);
	EXPECT_EQ(std::make_pair(2U, 0U), fragmentProcessor.inputs[0].memberLocations[1]);
}

TEST_F(SpirVProcessorTest, ExplicitInputsOutputsVarying)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"ExplicitInputsOutputsVarying.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_TRUE(fragmentProcessor.linkInputs(output, vertexProcessor));

	EXPECT_TRUE(output.getMessages().empty());

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(5U, vertexProcessor.outputs.size());

	EXPECT_EQ("floatVal", vertexProcessor.outputs[0].name);
	EXPECT_EQ(Type::Float, vertexProcessor.outputs[0].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[0].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[0].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].component);

	EXPECT_EQ("vec2Val", vertexProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec2, vertexProcessor.outputs[1].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[1].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[1].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[1].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[1].location);
	EXPECT_EQ(2U, vertexProcessor.outputs[1].component);

	EXPECT_EQ("ivec3Val", vertexProcessor.outputs[2].name);
	EXPECT_EQ(Type::IVec3, vertexProcessor.outputs[2].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[2].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[2].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[2].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[2].patch);
	EXPECT_EQ(1U, vertexProcessor.outputs[2].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[2].component);

	EXPECT_EQ("intVal", vertexProcessor.outputs[3].name);
	EXPECT_EQ(Type::Int, vertexProcessor.outputs[3].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[3].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[3].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[3].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[3].patch);
	EXPECT_EQ(1U, vertexProcessor.outputs[3].location);
	EXPECT_EQ(3U, vertexProcessor.outputs[3].component);

	EXPECT_EQ("", vertexProcessor.outputs[4].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.outputs[4].type);
	EXPECT_EQ(1U, vertexProcessor.outputs[4].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[4].arrayElements.empty());
	ASSERT_EQ(2U, vertexProcessor.outputs[4].memberLocations.size());
	EXPECT_FALSE(vertexProcessor.outputs[4].patch);
	EXPECT_EQ(2U, vertexProcessor.outputs[4].location);
	EXPECT_EQ(unknown, vertexProcessor.outputs[4].component);

	ASSERT_EQ(2U, vertexProcessor.structs.size());
	EXPECT_EQ("VertFragBlock", vertexProcessor.structs[1].name);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].size);

	ASSERT_EQ(2U, vertexProcessor.structs[1].members.size());
	EXPECT_EQ("dvec3Val", vertexProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::DVec3, vertexProcessor.structs[1].members[0].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[0].rowMajor);
	EXPECT_EQ(std::make_pair(2U, 0U), vertexProcessor.outputs[4].memberLocations[0]);

	EXPECT_EQ("mat4Val", vertexProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Mat4, vertexProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[1].rowMajor);
	EXPECT_EQ(std::make_pair(4U, 0U), vertexProcessor.outputs[4].memberLocations[1]);

	ASSERT_EQ(5U, fragmentProcessor.inputs.size());

	EXPECT_EQ("floatVal", fragmentProcessor.inputs[0].name);
	EXPECT_EQ(Type::Float, fragmentProcessor.inputs[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[0].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[0].patch);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].component);

	EXPECT_EQ("vec2Val", fragmentProcessor.inputs[1].name);
	EXPECT_EQ(Type::Vec2, fragmentProcessor.inputs[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[1].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[1].patch);
	EXPECT_EQ(0U, fragmentProcessor.inputs[1].location);
	EXPECT_EQ(2U, fragmentProcessor.inputs[1].component);

	EXPECT_EQ("ivec3Val", fragmentProcessor.inputs[2].name);
	EXPECT_EQ(Type::IVec3, fragmentProcessor.inputs[2].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[2].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[2].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[2].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[2].patch);
	EXPECT_EQ(1U, fragmentProcessor.inputs[2].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[2].component);

	EXPECT_EQ("intVal", fragmentProcessor.inputs[3].name);
	EXPECT_EQ(Type::Int, fragmentProcessor.inputs[3].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[3].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[3].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[3].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[3].patch);
	EXPECT_EQ(1U, fragmentProcessor.inputs[3].location);
	EXPECT_EQ(3U, fragmentProcessor.inputs[3].component);

	EXPECT_EQ("", fragmentProcessor.inputs[4].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.inputs[4].type);
	EXPECT_EQ(0U, fragmentProcessor.inputs[4].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[4].arrayElements.empty());
	ASSERT_EQ(2U, fragmentProcessor.inputs[4].memberLocations.size());
	EXPECT_FALSE(fragmentProcessor.inputs[4].patch);
	EXPECT_EQ(2U, fragmentProcessor.inputs[4].location);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[4].component);

	ASSERT_EQ(1U, fragmentProcessor.structs.size());
	EXPECT_EQ("VertFragBlock", fragmentProcessor.structs[0].name);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].size);

	ASSERT_EQ(2U, fragmentProcessor.structs[0].members.size());
	EXPECT_EQ("dvec3Val", fragmentProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::DVec3, fragmentProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[0].rowMajor);
	EXPECT_EQ(std::make_pair(2U, 0U), fragmentProcessor.inputs[4].memberLocations[0]);

	EXPECT_EQ("mat4Val", fragmentProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Mat4, fragmentProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[1].rowMajor);
	EXPECT_EQ(std::make_pair(4U, 0U), fragmentProcessor.inputs[4].memberLocations[1]);
}

TEST_F(SpirVProcessorTest, LinkAllStages)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkAllStages.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV tessControlSpirv = Compiler::assemble(output, program,
		Stage::TessellationControl, pipeline);
	Compiler::SpirV tessEvalSpirv = Compiler::assemble(output, program,
		Stage::TessellationEvaluation, pipeline);
	Compiler::SpirV geometrySpirv = Compiler::assemble(output, program, Stage::Geometry, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor tessControlProcessor;
	EXPECT_TRUE(tessControlProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, tessControlSpirv, Stage::TessellationControl));

	SpirVProcessor tessEvalProcessor;
	EXPECT_TRUE(tessEvalProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, tessEvalSpirv, Stage::TessellationEvaluation));

	SpirVProcessor geometryProcessor;
	EXPECT_TRUE(geometryProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, geometrySpirv, Stage::Geometry));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignInputs(output));
	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_TRUE(tessControlProcessor.linkInputs(output, vertexProcessor));
	EXPECT_TRUE(tessControlProcessor.assignOutputs(output));
	EXPECT_TRUE(tessEvalProcessor.linkInputs(output, tessControlProcessor));
	EXPECT_TRUE(tessEvalProcessor.assignOutputs(output));
	EXPECT_TRUE(geometryProcessor.linkInputs(output, tessEvalProcessor));
	EXPECT_TRUE(geometryProcessor.assignOutputs(output));
	EXPECT_TRUE(fragmentProcessor.linkInputs(output, geometryProcessor));
	EXPECT_TRUE(fragmentProcessor.assignOutputs(output));

	EXPECT_TRUE(output.getMessages().empty());

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(4U, vertexProcessor.inputs.size());

	EXPECT_EQ("position", vertexProcessor.inputs[0].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.inputs[0].type);
	EXPECT_EQ(unknown, vertexProcessor.inputs[0].structIndex);
	EXPECT_TRUE(vertexProcessor.inputs[0].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.inputs[0].patch);
	EXPECT_EQ(0U, vertexProcessor.inputs[0].location);
	EXPECT_EQ(0U, vertexProcessor.inputs[0].component);

	EXPECT_EQ("normal", vertexProcessor.inputs[1].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.inputs[1].type);
	EXPECT_EQ(unknown, vertexProcessor.inputs[1].structIndex);
	EXPECT_TRUE(vertexProcessor.inputs[1].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.inputs[1].patch);
	EXPECT_EQ(1U, vertexProcessor.inputs[1].location);
	EXPECT_EQ(0U, vertexProcessor.inputs[1].component);

	EXPECT_EQ("texCoords", vertexProcessor.inputs[2].name);
	EXPECT_EQ(Type::Vec2, vertexProcessor.inputs[2].type);
	EXPECT_EQ(unknown, vertexProcessor.inputs[2].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{3}, vertexProcessor.inputs[2].arrayElements);
	EXPECT_TRUE(vertexProcessor.inputs[2].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.inputs[2].patch);
	EXPECT_EQ(2U, vertexProcessor.inputs[2].location);
	EXPECT_EQ(0U, vertexProcessor.inputs[2].component);

	EXPECT_EQ("colors", vertexProcessor.inputs[3].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.inputs[3].type);
	EXPECT_EQ(unknown, vertexProcessor.inputs[3].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{2}, vertexProcessor.inputs[3].arrayElements);
	EXPECT_TRUE(vertexProcessor.inputs[3].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.inputs[3].patch);
	EXPECT_EQ(5U, vertexProcessor.inputs[3].location);
	EXPECT_EQ(0U, vertexProcessor.inputs[3].component);

	ASSERT_EQ(3U, vertexProcessor.outputs.size());

	EXPECT_EQ("vtcVertInfo", vertexProcessor.outputs[0].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.outputs[0].type);
	EXPECT_EQ(1U, vertexProcessor.outputs[0].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[0].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].component);

	EXPECT_EQ("vertShadowPos", vertexProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.outputs[1].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[1].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{4}, vertexProcessor.outputs[1].arrayElements);
	EXPECT_TRUE(vertexProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[1].patch);
	EXPECT_EQ(7U, vertexProcessor.outputs[1].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[1].component);

	EXPECT_EQ("vertFogVal", vertexProcessor.outputs[2].name);
	EXPECT_EQ(Type::Float, vertexProcessor.outputs[2].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[2].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[2].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[2].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[2].patch);
	EXPECT_EQ(11U, vertexProcessor.outputs[2].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[2].component);

	ASSERT_EQ(2U, vertexProcessor.structs.size());
	EXPECT_EQ("VertInfo", vertexProcessor.structs[1].name);
	ASSERT_EQ(4U, vertexProcessor.structs[1].members.size());

	EXPECT_EQ("position", vertexProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.structs[1].members[0].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[0].rowMajor);

	EXPECT_EQ("normal", vertexProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[1].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[1].rowMajor);

	EXPECT_EQ("texCoords", vertexProcessor.structs[1].members[2].name);
	EXPECT_EQ(Type::Vec2, vertexProcessor.structs[1].members[2].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, unknown}}),
		vertexProcessor.structs[1].members[2].arrayElements);
	EXPECT_FALSE(vertexProcessor.structs[1].members[2].rowMajor);

	EXPECT_EQ("colors", vertexProcessor.structs[1].members[3].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.structs[1].members[3].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[3].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, unknown}}),
		vertexProcessor.structs[1].members[3].arrayElements);
	EXPECT_FALSE(vertexProcessor.structs[1].members[3].rowMajor);

	ASSERT_EQ(3U, tessControlProcessor.inputs.size());

	EXPECT_EQ("vtcVertInfo", tessControlProcessor.inputs[0].name);
	EXPECT_EQ(Type::Struct, tessControlProcessor.inputs[0].type);
	EXPECT_EQ(0U, tessControlProcessor.inputs[0].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{32}, tessControlProcessor.inputs[0].arrayElements);
	EXPECT_TRUE(tessControlProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(tessControlProcessor.inputs[0].patch);
	EXPECT_EQ(0U, tessControlProcessor.inputs[0].location);
	EXPECT_EQ(0U, tessControlProcessor.inputs[0].component);

	EXPECT_EQ("vertShadowPos", tessControlProcessor.inputs[1].name);
	EXPECT_EQ(Type::Vec4, tessControlProcessor.inputs[1].type);
	EXPECT_EQ(unknown, tessControlProcessor.inputs[1].structIndex);
	EXPECT_EQ((std::vector<std::uint32_t>{32, 4}),
		tessControlProcessor.inputs[1].arrayElements);
	EXPECT_TRUE(tessControlProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(tessControlProcessor.inputs[1].patch);
	EXPECT_EQ(7U, tessControlProcessor.inputs[1].location);
	EXPECT_EQ(0U, tessControlProcessor.inputs[1].component);

	EXPECT_EQ("vertFogVal", tessControlProcessor.inputs[2].name);
	EXPECT_EQ(Type::Float, tessControlProcessor.inputs[2].type);
	EXPECT_EQ(unknown, tessControlProcessor.inputs[2].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{32}, tessControlProcessor.inputs[2].arrayElements);
	EXPECT_TRUE(tessControlProcessor.inputs[2].memberLocations.empty());
	EXPECT_FALSE(tessControlProcessor.inputs[2].patch);
	EXPECT_EQ(11U, tessControlProcessor.inputs[2].location);
	EXPECT_EQ(0U, tessControlProcessor.inputs[2].component);

	ASSERT_EQ(1U, tessControlProcessor.structs.size());
	EXPECT_EQ("VertInfo", tessControlProcessor.structs[0].name);
	ASSERT_EQ(4U, tessControlProcessor.structs[0].members.size());

	EXPECT_EQ("position", tessControlProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Vec3, tessControlProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, tessControlProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(tessControlProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(tessControlProcessor.structs[0].members[0].rowMajor);

	EXPECT_EQ("normal", tessControlProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec3, tessControlProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, tessControlProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(tessControlProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(tessControlProcessor.structs[0].members[1].rowMajor);

	EXPECT_EQ("texCoords", tessControlProcessor.structs[0].members[2].name);
	EXPECT_EQ(Type::Vec2, tessControlProcessor.structs[0].members[2].type);
	EXPECT_EQ(unknown, tessControlProcessor.structs[0].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, unknown}}),
		tessControlProcessor.structs[0].members[2].arrayElements);
	EXPECT_FALSE(tessControlProcessor.structs[0].members[2].rowMajor);

	EXPECT_EQ("colors", tessControlProcessor.structs[0].members[3].name);
	EXPECT_EQ(Type::Vec4, tessControlProcessor.structs[0].members[3].type);
	EXPECT_EQ(unknown, tessControlProcessor.structs[0].members[3].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, unknown}}),
		tessControlProcessor.structs[0].members[3].arrayElements);
	EXPECT_FALSE(tessControlProcessor.structs[0].members[3].rowMajor);

	ASSERT_EQ(4U, tessControlProcessor.outputs.size());

	EXPECT_EQ("tceVertInfo", tessControlProcessor.outputs[0].name);
	EXPECT_EQ(Type::Struct, tessControlProcessor.outputs[0].type);
	EXPECT_EQ(0U, tessControlProcessor.outputs[0].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{4}, tessControlProcessor.outputs[0].arrayElements);
	EXPECT_TRUE(tessControlProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(tessControlProcessor.outputs[0].patch);
	EXPECT_EQ(0U, tessControlProcessor.outputs[0].location);
	EXPECT_EQ(0U, tessControlProcessor.outputs[0].component);

	EXPECT_EQ("basis", tessControlProcessor.outputs[1].name);
	EXPECT_EQ(Type::Mat4, tessControlProcessor.outputs[1].type);
	EXPECT_EQ(unknown, tessControlProcessor.outputs[1].structIndex);
	EXPECT_TRUE(tessControlProcessor.outputs[1].arrayElements.empty());
	EXPECT_TRUE(tessControlProcessor.outputs[1].memberLocations.empty());
	EXPECT_TRUE(tessControlProcessor.outputs[1].patch);
	EXPECT_EQ(7U, tessControlProcessor.outputs[1].location);
	EXPECT_EQ(0U, tessControlProcessor.outputs[1].component);

	EXPECT_EQ("tessControlShadowPos", tessControlProcessor.outputs[2].name);
	EXPECT_EQ(Type::Vec4, tessControlProcessor.outputs[2].type);
	EXPECT_EQ(unknown, tessControlProcessor.outputs[2].structIndex);
	EXPECT_EQ((std::vector<std::uint32_t>{4, 4}),
		tessControlProcessor.outputs[2].arrayElements);
	EXPECT_TRUE(tessControlProcessor.outputs[2].memberLocations.empty());
	EXPECT_FALSE(tessControlProcessor.outputs[2].patch);
	EXPECT_EQ(11U, tessControlProcessor.outputs[2].location);
	EXPECT_EQ(0U, tessControlProcessor.outputs[2].component);

	EXPECT_EQ("tessControlFogVal", tessControlProcessor.outputs[3].name);
	EXPECT_EQ(Type::Float, tessControlProcessor.outputs[3].type);
	EXPECT_EQ(unknown, tessControlProcessor.outputs[3].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{4}, tessControlProcessor.outputs[3].arrayElements);
	EXPECT_TRUE(tessControlProcessor.outputs[3].memberLocations.empty());
	EXPECT_FALSE(tessControlProcessor.outputs[3].patch);
	EXPECT_EQ(15U, tessControlProcessor.outputs[3].location);
	EXPECT_EQ(0U, tessControlProcessor.outputs[3].component);

	ASSERT_EQ(4U, tessEvalProcessor.inputs.size());

	EXPECT_EQ("tceVertInfo", tessEvalProcessor.inputs[0].name);
	EXPECT_EQ(Type::Struct, tessEvalProcessor.inputs[0].type);
	EXPECT_EQ(0U, tessEvalProcessor.inputs[0].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{32}, tessEvalProcessor.inputs[0].arrayElements);
	EXPECT_TRUE(tessEvalProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(tessEvalProcessor.inputs[0].patch);
	EXPECT_EQ(0U, tessEvalProcessor.inputs[0].location);
	EXPECT_EQ(0U, tessEvalProcessor.inputs[0].component);

	EXPECT_EQ("basis", tessEvalProcessor.inputs[1].name);
	EXPECT_EQ(Type::Mat4, tessEvalProcessor.inputs[1].type);
	EXPECT_EQ(unknown, tessEvalProcessor.inputs[1].structIndex);
	EXPECT_TRUE(tessEvalProcessor.inputs[1].arrayElements.empty());
	EXPECT_TRUE(tessEvalProcessor.inputs[1].memberLocations.empty());
	EXPECT_TRUE(tessEvalProcessor.inputs[1].patch);
	EXPECT_EQ(7U, tessEvalProcessor.inputs[1].location);
	EXPECT_EQ(0U, tessEvalProcessor.inputs[1].component);

	EXPECT_EQ("tessControlShadowPos", tessEvalProcessor.inputs[2].name);
	EXPECT_EQ(Type::Vec4, tessEvalProcessor.inputs[2].type);
	EXPECT_EQ(unknown, tessEvalProcessor.inputs[2].structIndex);
	EXPECT_EQ((std::vector<std::uint32_t>{32, 4}),
		tessEvalProcessor.inputs[2].arrayElements);
	EXPECT_TRUE(tessEvalProcessor.inputs[2].memberLocations.empty());
	EXPECT_FALSE(tessEvalProcessor.inputs[2].patch);
	EXPECT_EQ(11U, tessEvalProcessor.inputs[2].location);
	EXPECT_EQ(0U, tessEvalProcessor.inputs[2].component);

	EXPECT_EQ("tessControlFogVal", tessEvalProcessor.inputs[3].name);
	EXPECT_EQ(Type::Float, tessEvalProcessor.inputs[3].type);
	EXPECT_EQ(unknown, tessEvalProcessor.inputs[3].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{32}, tessEvalProcessor.inputs[3].arrayElements);
	EXPECT_TRUE(tessEvalProcessor.inputs[3].memberLocations.empty());
	EXPECT_FALSE(tessEvalProcessor.inputs[3].patch);
	EXPECT_EQ(15U, tessEvalProcessor.inputs[3].location);
	EXPECT_EQ(0U, tessEvalProcessor.inputs[3].component);

	ASSERT_EQ(1U, tessEvalProcessor.structs.size());
	EXPECT_EQ("VertInfo", tessEvalProcessor.structs[0].name);
	ASSERT_EQ(4U, tessEvalProcessor.structs[0].members.size());

	EXPECT_EQ("position", tessEvalProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Vec3, tessEvalProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, tessEvalProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(tessEvalProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(tessEvalProcessor.structs[0].members[0].rowMajor);

	EXPECT_EQ("normal", tessEvalProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec3, tessEvalProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, tessEvalProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(tessEvalProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(tessEvalProcessor.structs[0].members[1].rowMajor);

	EXPECT_EQ("texCoords", tessEvalProcessor.structs[0].members[2].name);
	EXPECT_EQ(Type::Vec2, tessEvalProcessor.structs[0].members[2].type);
	EXPECT_EQ(unknown, tessEvalProcessor.structs[0].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, unknown}}),
		tessEvalProcessor.structs[0].members[2].arrayElements);
	EXPECT_FALSE(tessEvalProcessor.structs[0].members[2].rowMajor);

	EXPECT_EQ("colors", tessEvalProcessor.structs[0].members[3].name);
	EXPECT_EQ(Type::Vec4, tessEvalProcessor.structs[0].members[3].type);
	EXPECT_EQ(unknown, tessEvalProcessor.structs[0].members[3].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, unknown}}),
		tessEvalProcessor.structs[0].members[3].arrayElements);
	EXPECT_FALSE(tessEvalProcessor.structs[0].members[3].rowMajor);

	ASSERT_EQ(3U, tessEvalProcessor.outputs.size());

	EXPECT_EQ("tegVertInfo", tessEvalProcessor.outputs[0].name);
	EXPECT_EQ(Type::Struct, tessEvalProcessor.outputs[0].type);
	EXPECT_EQ(0U, tessEvalProcessor.outputs[0].structIndex);
	EXPECT_TRUE(tessEvalProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(tessEvalProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(tessEvalProcessor.outputs[0].patch);
	EXPECT_EQ(0U, tessEvalProcessor.outputs[0].location);
	EXPECT_EQ(0U, tessEvalProcessor.outputs[0].component);

	EXPECT_EQ("tessEvalShadowPos", tessEvalProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec4, tessEvalProcessor.outputs[1].type);
	EXPECT_EQ(unknown, tessEvalProcessor.outputs[1].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{4}, tessEvalProcessor.outputs[1].arrayElements);
	EXPECT_TRUE(tessEvalProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(tessEvalProcessor.outputs[1].patch);
	EXPECT_EQ(7U, tessEvalProcessor.outputs[1].location);
	EXPECT_EQ(0U, tessEvalProcessor.outputs[1].component);

	EXPECT_EQ("tessEvalFogVal", tessEvalProcessor.outputs[2].name);
	EXPECT_EQ(Type::Float, tessEvalProcessor.outputs[2].type);
	EXPECT_EQ(unknown, tessEvalProcessor.outputs[2].structIndex);
	EXPECT_TRUE(tessEvalProcessor.outputs[2].arrayElements.empty());
	EXPECT_TRUE(tessEvalProcessor.outputs[2].memberLocations.empty());
	EXPECT_FALSE(tessEvalProcessor.outputs[2].patch);
	EXPECT_EQ(11U, tessEvalProcessor.outputs[2].location);
	EXPECT_EQ(0U, tessEvalProcessor.outputs[2].component);

	ASSERT_EQ(3U, geometryProcessor.inputs.size());

	EXPECT_EQ("tegVertInfo", geometryProcessor.inputs[0].name);
	EXPECT_EQ(Type::Struct, geometryProcessor.inputs[0].type);
	EXPECT_EQ(0U, geometryProcessor.inputs[0].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{3}, geometryProcessor.inputs[0].arrayElements);
	EXPECT_TRUE(geometryProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(geometryProcessor.inputs[0].patch);
	EXPECT_EQ(0U, geometryProcessor.inputs[0].location);
	EXPECT_EQ(0U, geometryProcessor.inputs[0].component);

	EXPECT_EQ("tessEvalShadowPos", geometryProcessor.inputs[1].name);
	EXPECT_EQ(Type::Vec4, geometryProcessor.inputs[1].type);
	EXPECT_EQ(unknown, geometryProcessor.inputs[1].structIndex);
	EXPECT_EQ((std::vector<std::uint32_t>{3, 4}),
		geometryProcessor.inputs[1].arrayElements);
	EXPECT_TRUE(geometryProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(geometryProcessor.inputs[1].patch);
	EXPECT_EQ(7U, geometryProcessor.inputs[1].location);
	EXPECT_EQ(0U, geometryProcessor.inputs[1].component);

	EXPECT_EQ("tessEvalFogVal", geometryProcessor.inputs[2].name);
	EXPECT_EQ(Type::Float, geometryProcessor.inputs[2].type);
	EXPECT_EQ(unknown, geometryProcessor.inputs[2].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{3}, geometryProcessor.inputs[2].arrayElements);
	EXPECT_TRUE(geometryProcessor.inputs[2].memberLocations.empty());
	EXPECT_FALSE(geometryProcessor.inputs[2].patch);
	EXPECT_EQ(11U, geometryProcessor.inputs[2].location);
	EXPECT_EQ(0U, geometryProcessor.inputs[2].component);

	ASSERT_EQ(1U, geometryProcessor.structs.size());
	EXPECT_EQ("VertInfo", geometryProcessor.structs[0].name);
	ASSERT_EQ(4U, geometryProcessor.structs[0].members.size());

	EXPECT_EQ("position", geometryProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Vec3, geometryProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, geometryProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(geometryProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(geometryProcessor.structs[0].members[0].rowMajor);

	EXPECT_EQ("normal", geometryProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec3, geometryProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, geometryProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(geometryProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(geometryProcessor.structs[0].members[1].rowMajor);

	EXPECT_EQ("texCoords", geometryProcessor.structs[0].members[2].name);
	EXPECT_EQ(Type::Vec2, geometryProcessor.structs[0].members[2].type);
	EXPECT_EQ(unknown, geometryProcessor.structs[0].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, unknown}}),
		geometryProcessor.structs[0].members[2].arrayElements);
	EXPECT_FALSE(geometryProcessor.structs[0].members[2].rowMajor);

	EXPECT_EQ("colors", geometryProcessor.structs[0].members[3].name);
	EXPECT_EQ(Type::Vec4, geometryProcessor.structs[0].members[3].type);
	EXPECT_EQ(unknown, geometryProcessor.structs[0].members[3].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, unknown}}),
		geometryProcessor.structs[0].members[3].arrayElements);
	EXPECT_FALSE(geometryProcessor.structs[0].members[3].rowMajor);

	ASSERT_EQ(3U, geometryProcessor.outputs.size());

	EXPECT_EQ("gfVertInfo", geometryProcessor.outputs[0].name);
	EXPECT_EQ(Type::Struct, geometryProcessor.outputs[0].type);
	EXPECT_EQ(0U, geometryProcessor.outputs[0].structIndex);
	EXPECT_TRUE(geometryProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(geometryProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(geometryProcessor.outputs[0].patch);
	EXPECT_EQ(0U, geometryProcessor.outputs[0].location);
	EXPECT_EQ(0U, geometryProcessor.outputs[0].component);

	EXPECT_EQ("geomShadowPos", geometryProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec4, geometryProcessor.outputs[1].type);
	EXPECT_EQ(unknown, geometryProcessor.outputs[1].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{4}, geometryProcessor.outputs[1].arrayElements);
	EXPECT_TRUE(geometryProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(geometryProcessor.outputs[1].patch);
	EXPECT_EQ(7U, geometryProcessor.outputs[1].location);
	EXPECT_EQ(0U, geometryProcessor.outputs[1].component);

	EXPECT_EQ("geomFogVal", geometryProcessor.outputs[2].name);
	EXPECT_EQ(Type::Float, geometryProcessor.outputs[2].type);
	EXPECT_EQ(unknown, geometryProcessor.outputs[2].structIndex);
	EXPECT_TRUE(geometryProcessor.outputs[2].arrayElements.empty());
	EXPECT_TRUE(geometryProcessor.outputs[2].memberLocations.empty());
	EXPECT_FALSE(geometryProcessor.outputs[2].patch);
	EXPECT_EQ(11U, geometryProcessor.outputs[2].location);
	EXPECT_EQ(0U, geometryProcessor.outputs[2].component);

	ASSERT_EQ(3U, fragmentProcessor.inputs.size());

	EXPECT_EQ("gfVertInfo", fragmentProcessor.inputs[0].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.inputs[0].type);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[0].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[0].patch);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].component);

	EXPECT_EQ("geomShadowPos", fragmentProcessor.inputs[1].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.inputs[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[1].structIndex);
	EXPECT_EQ(std::vector<std::uint32_t>{4}, fragmentProcessor.inputs[1].arrayElements);
	EXPECT_TRUE(fragmentProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[1].patch);
	EXPECT_EQ(7U, fragmentProcessor.inputs[1].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[1].component);

	EXPECT_EQ("geomFogVal", fragmentProcessor.inputs[2].name);
	EXPECT_EQ(Type::Float, fragmentProcessor.inputs[2].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[2].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[2].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[2].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[2].patch);
	EXPECT_EQ(11U, fragmentProcessor.inputs[2].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[2].component);

	ASSERT_EQ(1U, fragmentProcessor.structs.size());
	EXPECT_EQ("VertInfo", fragmentProcessor.structs[0].name);
	ASSERT_EQ(4U, fragmentProcessor.structs[0].members.size());

	EXPECT_EQ("position", fragmentProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Vec3, fragmentProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[0].rowMajor);

	EXPECT_EQ("normal", fragmentProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec3, fragmentProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[1].rowMajor);

	EXPECT_EQ("texCoords", fragmentProcessor.structs[0].members[2].name);
	EXPECT_EQ(Type::Vec2, fragmentProcessor.structs[0].members[2].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[2].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{3, unknown}}),
		fragmentProcessor.structs[0].members[2].arrayElements);
	EXPECT_FALSE(fragmentProcessor.structs[0].members[2].rowMajor);

	EXPECT_EQ("colors", fragmentProcessor.structs[0].members[3].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.structs[0].members[3].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[3].structIndex);
	EXPECT_EQ((std::vector<ArrayInfo>{{2, unknown}}),
		fragmentProcessor.structs[0].members[3].arrayElements);
	EXPECT_FALSE(fragmentProcessor.structs[0].members[3].rowMajor);

	ASSERT_EQ(1U, fragmentProcessor.outputs.size());
	EXPECT_EQ("color", fragmentProcessor.outputs[0].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.outputs[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.outputs[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.outputs[0].patch);
	EXPECT_EQ(0U, fragmentProcessor.outputs[0].location);
	EXPECT_EQ(0U, fragmentProcessor.outputs[0].component);
}

TEST_F(SpirVProcessorTest, LinkStructArray)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkStructArray.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_TRUE(fragmentProcessor.linkInputs(output, vertexProcessor));

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(2U, vertexProcessor.outputs.size());

	EXPECT_EQ("inouts", vertexProcessor.outputs[0].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.outputs[0].type);
	EXPECT_EQ(1U, vertexProcessor.outputs[0].structIndex);
	ASSERT_EQ(1U, vertexProcessor.outputs[0].arrayElements.size());
	EXPECT_EQ(2U, vertexProcessor.outputs[0].arrayElements[0]);
	EXPECT_TRUE(vertexProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[0].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].component);

	EXPECT_EQ("otherValue", vertexProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.outputs[1].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[1].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[1].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[1].patch);
	EXPECT_EQ(8U, vertexProcessor.outputs[1].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[1].component);

	ASSERT_EQ(2U, vertexProcessor.structs.size());
	EXPECT_EQ("InOuts", vertexProcessor.structs[1].name);
	ASSERT_EQ(2U, vertexProcessor.structs[1].members.size());

	EXPECT_EQ("vec4Value", vertexProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.structs[1].members[0].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[0].rowMajor);

	EXPECT_EQ("vec3Values", vertexProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[1].structIndex);
	ASSERT_EQ(1U, vertexProcessor.structs[1].members[1].arrayElements.size());
	EXPECT_EQ(3U, vertexProcessor.structs[1].members[1].arrayElements[0].length);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[1].arrayElements[0].stride);
	EXPECT_FALSE(vertexProcessor.structs[1].members[1].rowMajor);

	ASSERT_EQ(2U, fragmentProcessor.inputs.size());

	EXPECT_EQ("otherValue", fragmentProcessor.inputs[0].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.inputs[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[0].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[0].patch);
	EXPECT_EQ(8U, fragmentProcessor.inputs[0].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].component);

	EXPECT_EQ("inouts", fragmentProcessor.inputs[1].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.inputs[1].type);
	EXPECT_EQ(0U, fragmentProcessor.inputs[1].structIndex);
	ASSERT_EQ(1U, fragmentProcessor.inputs[1].arrayElements.size());
	EXPECT_EQ(2U, fragmentProcessor.inputs[1].arrayElements[0]);
	EXPECT_TRUE(fragmentProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[1].patch);
	EXPECT_EQ(0U, fragmentProcessor.inputs[1].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[1].component);

	ASSERT_EQ(1U, fragmentProcessor.structs.size());
	EXPECT_EQ("InOuts", fragmentProcessor.structs[0].name);
	ASSERT_EQ(2U, fragmentProcessor.structs[0].members.size());

	EXPECT_EQ("vec4Value", fragmentProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[0].rowMajor);

	EXPECT_EQ("vec3Values", fragmentProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec3, fragmentProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[1].structIndex);
	ASSERT_EQ(1U, fragmentProcessor.structs[0].members[1].arrayElements.size());
	EXPECT_EQ(3U, fragmentProcessor.structs[0].members[1].arrayElements[0].length);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[1].arrayElements[0].stride);
	EXPECT_FALSE(fragmentProcessor.structs[0].members[1].rowMajor);
}

TEST_F(SpirVProcessorTest, LinkStructWithinOutputBlock)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkStructWithinOutputBlock.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_TRUE(fragmentProcessor.linkInputs(output, vertexProcessor));

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(1U, vertexProcessor.outputs.size());

	EXPECT_EQ("", vertexProcessor.outputs[0].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.outputs[0].type);
	EXPECT_EQ(2U, vertexProcessor.outputs[0].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[0].arrayElements.empty());
	ASSERT_EQ(2U, vertexProcessor.outputs[0].memberLocations.size());
	EXPECT_FALSE(vertexProcessor.outputs[0].patch);
	EXPECT_EQ(unknown, vertexProcessor.outputs[0].location);
	EXPECT_EQ(unknown, vertexProcessor.outputs[0].component);

	ASSERT_EQ(3U, vertexProcessor.structs.size());
	EXPECT_EQ("OutputStruct", vertexProcessor.structs[1].name);
	ASSERT_EQ(2U, vertexProcessor.structs[1].members.size());

	EXPECT_EQ("firstValue", vertexProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.structs[1].members[0].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[0].rowMajor);

	EXPECT_EQ("secondValue", vertexProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[1].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[1].rowMajor);

	EXPECT_EQ("OutValues", vertexProcessor.structs[2].name);
	ASSERT_EQ(2U, vertexProcessor.structs[2].members.size());

	EXPECT_EQ("structValue", vertexProcessor.structs[2].members[0].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.structs[2].members[0].type);
	EXPECT_EQ(1U, vertexProcessor.structs[2].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[2].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[2].members[0].rowMajor);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].memberLocations[0].first);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].memberLocations[0].second);

	EXPECT_EQ("vec4Value", vertexProcessor.structs[2].members[1].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.structs[2].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[2].members[1].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[2].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[2].members[1].rowMajor);
	EXPECT_EQ(2U, vertexProcessor.outputs[0].memberLocations[1].first);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].memberLocations[1].second);

	ASSERT_EQ(1U, fragmentProcessor.inputs.size());

	EXPECT_EQ("", fragmentProcessor.inputs[0].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.inputs[0].type);
	EXPECT_EQ(1U, fragmentProcessor.inputs[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[0].arrayElements.empty());
	ASSERT_EQ(2U, fragmentProcessor.inputs[0].memberLocations.size());
	EXPECT_FALSE(fragmentProcessor.inputs[0].patch);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[0].location);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[0].component);

	ASSERT_EQ(2U, fragmentProcessor.structs.size());
	EXPECT_EQ("OutputStruct", fragmentProcessor.structs[0].name);
	ASSERT_EQ(2U, fragmentProcessor.structs[0].members.size());

	EXPECT_EQ("firstValue", fragmentProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[0].rowMajor);

	EXPECT_EQ("secondValue", fragmentProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec3, fragmentProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[1].rowMajor);

	EXPECT_EQ("InValues", fragmentProcessor.structs[1].name);
	ASSERT_EQ(2U, fragmentProcessor.structs[1].members.size());

	EXPECT_EQ("structValue", fragmentProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.structs[1].members[0].type);
	EXPECT_EQ(0U, fragmentProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[1].members[0].rowMajor);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].memberLocations[0].first);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].memberLocations[0].second);

	EXPECT_EQ("vec4Value", fragmentProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[1].members[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[1].members[1].rowMajor);
	EXPECT_EQ(2U, fragmentProcessor.inputs[0].memberLocations[1].first);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].memberLocations[1].second);
}

TEST_F(SpirVProcessorTest, LinkNestedStructs)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkNestedStructs.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_TRUE(fragmentProcessor.linkInputs(output, vertexProcessor));

	std::uint32_t unknown = msl::compile::unknown;
	ASSERT_EQ(2U, vertexProcessor.outputs.size());

	EXPECT_EQ("structValues", vertexProcessor.outputs[0].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.outputs[0].type);
	EXPECT_EQ(2U, vertexProcessor.outputs[0].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[0].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[0].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[0].patch);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[0].component);

	EXPECT_EQ("otherValue", vertexProcessor.outputs[1].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.outputs[1].type);
	EXPECT_EQ(unknown, vertexProcessor.outputs[1].structIndex);
	EXPECT_TRUE(vertexProcessor.outputs[1].arrayElements.empty());
	EXPECT_TRUE(vertexProcessor.outputs[1].memberLocations.empty());
	EXPECT_FALSE(vertexProcessor.outputs[1].patch);
	EXPECT_EQ(3U, vertexProcessor.outputs[1].location);
	EXPECT_EQ(0U, vertexProcessor.outputs[1].component);

	ASSERT_EQ(3U, vertexProcessor.structs.size());
	EXPECT_EQ("InnerStruct", vertexProcessor.structs[1].name);
	ASSERT_EQ(2U, vertexProcessor.structs[1].members.size());

	EXPECT_EQ("firstValue", vertexProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.structs[1].members[0].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[0].rowMajor);

	EXPECT_EQ("secondValue", vertexProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Vec3, vertexProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[1].members[1].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[1].members[1].rowMajor);

	EXPECT_EQ("OuterStruct", vertexProcessor.structs[2].name);
	ASSERT_EQ(2U, vertexProcessor.structs[2].members.size());

	EXPECT_EQ("structValue", vertexProcessor.structs[2].members[0].name);
	EXPECT_EQ(Type::Struct, vertexProcessor.structs[2].members[0].type);
	EXPECT_EQ(1U, vertexProcessor.structs[2].members[0].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[2].members[0].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[2].members[0].rowMajor);

	EXPECT_EQ("vec4Value", vertexProcessor.structs[2].members[1].name);
	EXPECT_EQ(Type::Vec4, vertexProcessor.structs[2].members[1].type);
	EXPECT_EQ(unknown, vertexProcessor.structs[2].members[1].structIndex);
	EXPECT_TRUE(vertexProcessor.structs[2].members[1].arrayElements.empty());
	EXPECT_FALSE(vertexProcessor.structs[2].members[1].rowMajor);

	ASSERT_EQ(2U, fragmentProcessor.inputs.size());

	EXPECT_EQ("otherValue", fragmentProcessor.inputs[0].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.inputs[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.inputs[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[0].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[0].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[0].patch);
	EXPECT_EQ(3U, fragmentProcessor.inputs[0].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[0].component);

	EXPECT_EQ("structValues", fragmentProcessor.inputs[1].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.inputs[1].type);
	EXPECT_EQ(1U, fragmentProcessor.inputs[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.inputs[1].arrayElements.empty());
	EXPECT_TRUE(fragmentProcessor.inputs[1].memberLocations.empty());
	EXPECT_FALSE(fragmentProcessor.inputs[1].patch);
	EXPECT_EQ(0U, fragmentProcessor.inputs[1].location);
	EXPECT_EQ(0U, fragmentProcessor.inputs[1].component);

	ASSERT_EQ(2U, fragmentProcessor.structs.size());
	EXPECT_EQ("InnerStruct", fragmentProcessor.structs[0].name);
	ASSERT_EQ(2U, fragmentProcessor.structs[0].members.size());

	EXPECT_EQ("firstValue", fragmentProcessor.structs[0].members[0].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.structs[0].members[0].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[0].rowMajor);

	EXPECT_EQ("secondValue", fragmentProcessor.structs[0].members[1].name);
	EXPECT_EQ(Type::Vec3, fragmentProcessor.structs[0].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[0].members[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[0].members[1].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[0].members[1].rowMajor);

	EXPECT_EQ("OuterStruct", fragmentProcessor.structs[1].name);
	ASSERT_EQ(2U, fragmentProcessor.structs[1].members.size());

	EXPECT_EQ("structValue", fragmentProcessor.structs[1].members[0].name);
	EXPECT_EQ(Type::Struct, fragmentProcessor.structs[1].members[0].type);
	EXPECT_EQ(0U, fragmentProcessor.structs[1].members[0].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[1].members[0].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[1].members[0].rowMajor);

	EXPECT_EQ("vec4Value", fragmentProcessor.structs[1].members[1].name);
	EXPECT_EQ(Type::Vec4, fragmentProcessor.structs[1].members[1].type);
	EXPECT_EQ(unknown, fragmentProcessor.structs[1].members[1].structIndex);
	EXPECT_TRUE(fragmentProcessor.structs[1].members[1].arrayElements.empty());
	EXPECT_FALSE(fragmentProcessor.structs[1].members[1].rowMajor);
}

TEST_F(SpirVProcessorTest, ComputeLocalSize)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"ComputeLocalSize.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV computeSpirv = Compiler::assemble(output, program, Stage::Compute, pipeline);

	SpirVProcessor computeProcessor;
	EXPECT_TRUE(computeProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, computeSpirv, Stage::Compute));

	EXPECT_EQ(2U, computeProcessor.computeLocalSize[0]);
	EXPECT_EQ(3U, computeProcessor.computeLocalSize[1]);
	EXPECT_EQ(4U, computeProcessor.computeLocalSize[2]);
}

TEST_F(SpirVProcessorTest, ClipAndCullDistances)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"ClipAndCullDistances.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	EXPECT_EQ(3U, vertexProcessor.clipDistanceCount);
	EXPECT_EQ(4U, vertexProcessor.cullDistanceCount);
}

TEST_F(SpirVProcessorTest, LinkDifferentType)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkDifferentType.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_FALSE(fragmentProcessor.linkInputs(output, vertexProcessor));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(18U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("linker error: type mismatch when linking input value in stage fragment",
		messages[0].message);
}

TEST_F(SpirVProcessorTest, LinkDifferentInlineStructType)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkDifferentInlineStructType.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_FALSE(fragmentProcessor.linkInputs(output, vertexProcessor));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(25U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ(
		"linker error: type mismatch when linking input member InValues.value in stage fragment",
		messages[0].message);
}

TEST_F(SpirVProcessorTest, LinkDifferentStructType)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkDifferentStructType.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_FALSE(fragmentProcessor.linkInputs(output, vertexProcessor));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(41U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ(
		"linker error: type mismatch when linking input value in stage fragment",
		messages[0].message);
}

TEST_F(SpirVProcessorTest, LinkNotPresent)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkNotPresent.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_FALSE(fragmentProcessor.linkInputs(output, vertexProcessor));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(18U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("linker error: cannot find output with name otherValue in stage vertex",
		messages[0].message);
}

TEST_F(SpirVProcessorTest, LinkStructNotPresent)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"LinkStructNotPresent.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_FALSE(fragmentProcessor.linkInputs(output, vertexProcessor));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(25U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("linker error: cannot find output interface block member with name otherValue in "
		"stage vertex", messages[0].message);
}

TEST_F(SpirVProcessorTest, MultipleLinkMembers)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"MultipleLinkMembers.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_TRUE(vertexProcessor.assignOutputs(output));
	EXPECT_FALSE(fragmentProcessor.linkInputs(output, vertexProcessor));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(30U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("linker error: multiple members from output interface blocks match the name value in "
		"stage vertex", messages[0].message);
}

TEST_F(SpirVProcessorTest, MixedExplicitImplicitLocations)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"MixedExplicitImplicitLocations.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_TRUE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	SpirVProcessor fragmentProcessor;
	EXPECT_TRUE(fragmentProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, fragmentSpirv, Stage::Fragment));

	EXPECT_FALSE(vertexProcessor.assignOutputs(output));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(18U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("linker error: output declarations mix implicit and explicit locations in stage "
		"vertex", messages[0].message);
}

TEST_F(SpirVProcessorTest, StructInBlockAndBuffer)
{
	boost::filesystem::path inputDir = exeDir/"inputs";
	std::string shaderName = pathStr(inputDir/"StructInBlockAndBuffer.msl");

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
		if (pipeline.entryPoints[i].value.empty())
			continue;

		auto stage = static_cast<Stage>(i);
		std::vector<Parser::LineMapping> lineMappings;
		std::string glsl =
			parser.createShaderString(lineMappings, output, pipeline, stage, false, false);
		EXPECT_TRUE(Compiler::compile(stages, output, shaderName, glsl, lineMappings, stage,
			Compiler::getDefaultResources(), spirvVersion));
		compiledStage = true;
	}
	EXPECT_TRUE(compiledStage);

	Compiler::Program program;
	EXPECT_TRUE(Compiler::link(program, output, pipeline, stages));
	Compiler::SpirV vertexSpirv = Compiler::assemble(output, program, Stage::Vertex, pipeline);
	Compiler::SpirV fragmentSpirv = Compiler::assemble(output, program, Stage::Fragment, pipeline);

	SpirVProcessor vertexProcessor;
	EXPECT_FALSE(vertexProcessor.extract(output, pipeline.token->fileName, pipeline.token->line,
		pipeline.token->column, vertexSpirv, Stage::Vertex));

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_TRUE(boost::algorithm::ends_with(pathStr(messages[0].file), shaderName));
	EXPECT_EQ(30U, messages[0].line);
	EXPECT_EQ(10U, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("linker error: multiple sructs of name TestStruct declared; this could be due to "
		"using the same struct in different contexts, such a uniform block and uniform buffer",
		messages[0].message);
}

} // namespace msl
