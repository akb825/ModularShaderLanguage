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

#include <MSL/Compile/CompiledResult.h>
#include <MSL/Compile/Target.h>
#include "mslb_generated.h"
#include <fstream>

namespace msl
{

using namespace compile;

static_assert(static_cast<int>(mslb::Type::MAX) == typeCount - 1,
	"Type enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::UniformType::MAX) ==
	static_cast<int>(UniformType::SubpassInput),
	"UniformType enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::Bool::MAX) == static_cast<int>(Bool::True),
	"Bool enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::PolygonMode::MAX) == static_cast<int>(PolygonMode::Point),
	"PolygonMode enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::CullMode::MAX) == static_cast<int>(CullMode::FrontAndBack),
	"CullMode enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::FrontFace::MAX) == static_cast<int>(FrontFace::Clockwise),
	"FrontFace enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::StencilOp::MAX) ==
	static_cast<int>(StencilOp::DecrementAndWrap),
	"StencilOp enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::CompareOp::MAX) == static_cast<int>(CompareOp::Always),
	"CompareOp enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::BlendFactor::MAX) ==
	static_cast<int>(BlendFactor::OneMinusSrc1Alpha),
	"CullMode enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::BlendOp::MAX) == static_cast<int>(BlendOp::Max),
	"BlendOp enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::LogicOp::MAX) == static_cast<int>(LogicOp::Set),
	"LogicOp enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::Filter::MAX) == static_cast<int>(Filter::Linear),
	"Filter enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::MipFilter::MAX) == static_cast<int>(MipFilter::Anisotropic),
	"MipFilter enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::AddressMode::MAX) == static_cast<int>(AddressMode::MirrorOnce),
	"AddressMode enum mismatch between flatbuffer and C++.");
static_assert(static_cast<int>(mslb::BorderColor::MAX) ==
	static_cast<int>(BorderColor::OpaqueIntOne),
	"BorderColor enum mismatch between flatbuffer and C++.");

CompiledResult::CompiledResult()
	: m_target(nullptr)
{
}

std::uint32_t CompiledResult::getTargetId() const
{
	return m_target ? m_target->getId() : 0;
}

std::uint32_t CompiledResult::getTargetVersion() const
{
	return m_target ? m_target->getVersion() : 0;
}

std::size_t CompiledResult::addShader(std::vector<uint8_t> shader, bool dontRemoveDuplicates)
{
	if (!dontRemoveDuplicates)
	{
		for (std::size_t i = 0; i < m_shaders.size(); ++i)
		{
			if (m_shaders[i] == shader)
				return i;
		}
	}

	m_shaders.push_back(std::move(shader));
	return m_shaders.size() - 1;
}

bool CompiledResult::save(std::ostream& stream) const
{
	if (!m_target)
		return false;

	bool isSpirV = m_target->getId() == MSL_CREATE_ID('S', 'P', 'R', 'V');
	flatbuffers::FlatBufferBuilder builder;
	std::vector<flatbuffers::Offset<mslb::Pipeline>> pipelines(m_pipelines.size());
	std::vector<flatbuffers::Offset<mslb::Struct>> structs;
	std::vector<mslb::ArrayInfo> arrayElements;
	std::vector<flatbuffers::Offset<mslb::StructMember>> structMembers;
	std::vector<mslb::SamplerState> samplerStates;
	std::vector<flatbuffers::Offset<mslb::Uniform>> uniforms;
	std::vector<flatbuffers::Offset<mslb::Attribute>> attributes;
	std::vector<mslb::BlendAttachmentState> blendAttachments;
	blendAttachments.reserve(maxAttachments);
	std::vector<flatbuffers::Offset<mslb::Shader>> shaders(maxAttachments);

	std::size_t i = 0;
	for (const auto& pipeline : m_pipelines)
	{
		structs.resize(pipeline.second.structs.size());
		for (std::size_t j = 0; j < structs.size(); ++j)
		{
			const Struct& pipelineStruct = pipeline.second.structs[j];
			structMembers.resize(pipelineStruct.members.size());
			for (std::size_t k = 0; k < structMembers.size(); ++k)
			{
				const StructMember& member = pipelineStruct.members[k];
				arrayElements.clear();
				arrayElements.reserve(member.arrayElements.size());
				for (std::size_t l = 0; l < arrayElements.size(); ++l)
				{
					arrayElements.emplace_back(member.arrayElements[l].length,
						member.arrayElements[l].stride);
				}

				assert(member.structIndex == unknown || member.structIndex < structs.size());
				structMembers[k] = mslb::CreateStructMember(builder,
					builder.CreateString(member.name), member.offset, member.size,
					static_cast<mslb::Type>(member.type), member.structIndex,
					arrayElements.empty() ? 0 : builder.CreateVectorOfStructs(arrayElements));
			}

			structs[j] = mslb::CreateStruct(builder, builder.CreateString(pipelineStruct.name),
				pipelineStruct.size, builder.CreateVector(structMembers));

		}

		samplerStates.clear();
		samplerStates.reserve(pipeline.second.samplerStates.size());
		for (std::size_t j = 0; j < samplerStates.size(); ++j)
		{
			const SamplerState& samplerState = pipeline.second.samplerStates[j];
			samplerStates.emplace_back(static_cast<mslb::Filter>(samplerState.minFilter),
				static_cast<mslb::Filter>(samplerState.magFilter),
				static_cast<mslb::MipFilter>(samplerState.mipFilter),
				static_cast<mslb::AddressMode>(samplerState.addressModeU),
				static_cast<mslb::AddressMode>(samplerState.addressModeV),
				static_cast<mslb::AddressMode>(samplerState.addressModeW),
				samplerState.mipLodBias, samplerState.maxAnisotropy, samplerState.minLod,
				samplerState.maxLod, static_cast<mslb::BorderColor>(samplerState.borderColor));
		}

		uniforms.resize(pipeline.second.uniforms.size());
		for (std::size_t j = 0; j < uniforms.size(); ++j)
		{
			const Uniform& uniform = pipeline.second.uniforms[j];
			arrayElements.clear();
			arrayElements.reserve(uniform.arrayElements.size());
			for (std::size_t k = 0; k < arrayElements.size(); ++k)
			{
				arrayElements.emplace_back(uniform.arrayElements[k].length,
					uniform.arrayElements[k].stride);
			}

			assert(uniform.structIndex == unknown || uniform.structIndex < structs.size());
			assert(uniform.samplerIndex == unknown || uniform.samplerIndex < samplerStates.size());
			uniforms[j] = mslb::CreateUniform(builder, builder.CreateString(uniform.name),
				static_cast<mslb::UniformType>(uniform.uniformType),
				static_cast<mslb::Type>(uniform.type), uniform.structIndex,
				arrayElements.empty() ? 0 : builder.CreateVectorOfStructs(arrayElements),
				uniform.descriptorSet, uniform.binding, uniform.samplerIndex);
		}

		attributes.resize(pipeline.second.attributes.size());
		for (std::size_t j = 0; j < attributes.size(); ++j)
		{
			const Attribute& attribute = pipeline.second.attributes[j];
			arrayElements.clear();
			arrayElements.reserve(attribute.arrayElements.size());
			for (std::size_t k = 0; k < arrayElements.size(); ++k)
			{
				arrayElements.emplace_back(attribute.arrayElements[k].length,
					attribute.arrayElements[k].stride);
			}

			attributes[j] = mslb::CreateAttribute(builder, builder.CreateString(attribute.name),
				static_cast<mslb::Type>(attribute.type),
				arrayElements.empty() ? 0 : builder.CreateVectorOfStructs(arrayElements),
				attribute.location, attribute.component);
		}

		const RenderState& renderState = pipeline.second.renderState;
		mslb::RasterizationState rasterizationState(
			static_cast<mslb::Bool>(renderState.rasterizationState.depthClampEnable),
			static_cast<mslb::Bool>(renderState.rasterizationState.rasterizerDiscardEnable),
			static_cast<mslb::PolygonMode>(renderState.rasterizationState.polygonMode),
			static_cast<mslb::CullMode>(renderState.rasterizationState.cullMode),
			static_cast<mslb::FrontFace>(renderState.rasterizationState.frontFace),
			static_cast<mslb::Bool>(renderState.rasterizationState.depthBiasEnable),
			renderState.rasterizationState.depthBiasConstantFactor,
			renderState.rasterizationState.depthBiasClamp,
			renderState.rasterizationState.depthBiasSlopeFactor,
			renderState.rasterizationState.lineWidth);

		mslb::MultisampleState multisampleState(
			static_cast<mslb::Bool>(renderState.multisampleState.sampleShadingEnable),
			renderState.multisampleState.minSampleShading,
			renderState.multisampleState.sampleMask,
			static_cast<mslb::Bool>(renderState.multisampleState.alphaToCoverageEnable),
			static_cast<mslb::Bool>(renderState.multisampleState.alphaToOneEnable));

		mslb::StencilOpState frontStencilState(
			static_cast<mslb::StencilOp>(renderState.depthStencilState.frontStencil.failOp),
			static_cast<mslb::StencilOp>(renderState.depthStencilState.frontStencil.passOp),
			static_cast<mslb::StencilOp>(renderState.depthStencilState.frontStencil.depthFailOp),
			static_cast<mslb::CompareOp>(renderState.depthStencilState.frontStencil.compareOp),
			renderState.depthStencilState.frontStencil.compareMask,
			renderState.depthStencilState.frontStencil.writeMask,
			renderState.depthStencilState.frontStencil.reference);

		mslb::StencilOpState backStencilState(
			static_cast<mslb::StencilOp>(renderState.depthStencilState.backStencil.failOp),
			static_cast<mslb::StencilOp>(renderState.depthStencilState.backStencil.passOp),
			static_cast<mslb::StencilOp>(renderState.depthStencilState.backStencil.depthFailOp),
			static_cast<mslb::CompareOp>(renderState.depthStencilState.backStencil.compareOp),
			renderState.depthStencilState.backStencil.compareMask,
			renderState.depthStencilState.backStencil.writeMask,
			renderState.depthStencilState.backStencil.reference);

		mslb::DepthStencilState depthStencilState(
			static_cast<mslb::Bool>(renderState.depthStencilState.depthTestEnable),
			static_cast<mslb::Bool>(renderState.depthStencilState.depthWriteEnable),
			static_cast<mslb::CompareOp>(renderState.depthStencilState.depthCompareOp),
			static_cast<mslb::Bool>(renderState.depthStencilState.depthBoundsTestEnable),
			static_cast<mslb::Bool>(renderState.depthStencilState.stencilTestEnable),
			frontStencilState, backStencilState,
			renderState.depthStencilState.minDepthBounds,
			renderState.depthStencilState.maxDepthBounds);

		blendAttachments.clear();
		for (unsigned int j = 0; j < maxAttachments; ++j)
		{
			blendAttachments.emplace_back(
				static_cast<mslb::Bool>(renderState.blendState.blendAttachments[j].blendEnable),
				static_cast<mslb::BlendFactor>(
					renderState.blendState.blendAttachments[j].srcColorBlendFactor),
				static_cast<mslb::BlendFactor>(
					renderState.blendState.blendAttachments[j].dstColorBlendFactor),
				static_cast<mslb::BlendOp>(renderState.blendState.blendAttachments[j].colorBlendOp),
				static_cast<mslb::BlendFactor>(
					renderState.blendState.blendAttachments[j].srcAlphaBlendFactor),
				static_cast<mslb::BlendFactor>(
					renderState.blendState.blendAttachments[j].dstAlphaBlendFactor),
				static_cast<mslb::BlendOp>(renderState.blendState.blendAttachments[j].alphaBlendOp),
				static_cast<mslb::ColorMask>(
					renderState.blendState.blendAttachments[j].colorWriteMask));
		}

		flatbuffers::Offset<mslb::BlendState> blendState = mslb::CreateBlendState(builder,
			static_cast<mslb::Bool>(renderState.blendState.logicalOpEnable),
			static_cast<mslb::LogicOp>(renderState.blendState.logicalOp),
			static_cast<mslb::Bool>(renderState.blendState.separateAttachmentBlendingEnable),
			builder.CreateVectorOfStructs(blendAttachments),
			builder.CreateVector(renderState.blendState.blendConstants.data(), 4));

		for (unsigned int j = 0; j < maxAttachments; ++j)
		{
			const Shader& shader = pipeline.second.shaders[j];
			assert(shader.shader == unknown || shader.shader < m_shaders.size());
			shaders[j] = mslb::CreateShader(builder, static_cast<std::uint32_t>(shader.shader),
				shader.shader == unknown || !isSpirV ?
				0 : builder.CreateVector(shader.uniformIds));
		}

		assert(pipeline.second.pushConstantStruct == unknown ||
			pipeline.second.pushConstantStruct < structs.size());
		pipelines[i] = mslb::CreatePipeline(builder,
			builder.CreateVector(structs),
			builder.CreateVectorOfStructs(samplerStates),
			builder.CreateVector(uniforms),
			builder.CreateVector(attributes),
			pipeline.second.pushConstantStruct,
			mslb::CreateRenderState(builder, &rasterizationState, &multisampleState,
				&depthStencilState, blendState, renderState.patchControlPoints),
			builder.CreateVector(shaders));

		++i;
	}

	// Swap if big endian and SPIR-V since it uses 32-bit values.
	bool swap = !FLATBUFFERS_LITTLEENDIAN && isSpirV;
	std::vector<uint8_t> swapShader;
	std::vector<flatbuffers::Offset<mslb::ShaderData>> shaderData(m_shaders.size());
	for (i = 0; i < m_shaders.size(); ++i)
	{
		if (swap)
		{
			swapShader = m_shaders[i];
			std::uint32_t* swapShader32 = reinterpret_cast<std::uint32_t*>(swapShader.data());
			std::size_t swapShader32Size = swapShader.size()/sizeof(std::uint32_t);
			for (std::size_t j = 0; j < swapShader32Size; ++j)
				swapShader32[j] = flatbuffers::EndianScalar(swapShader[j]);
			shaderData[i] = mslb::CreateShaderData(builder, builder.CreateVector(swapShader));
		}
		else
			shaderData[i] = mslb::CreateShaderData(builder, builder.CreateVector(m_shaders[i]));
	}

	builder.Finish(mslb::CreateModule(builder,
		version,
		m_target->getId(),
		m_target->getVersion(),
		m_target->getAdjustableBindings() && isSpirV,
		builder.CreateVector(pipelines),
		builder.CreateVector(shaderData),
		builder.CreateVector(m_sharedData)));

	stream.write(reinterpret_cast<const char*>(builder.GetBufferPointer()), builder.GetSize());
	return true;
}

bool CompiledResult::save(const std::string& fileName) const
{
	std::ofstream stream(fileName, std::ios_base::trunc | std::ios_base::binary);
	if (!stream.is_open())
		return false;

	return save(stream);
}

} // namespace msl
