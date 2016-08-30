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

#pragma once

#include <MSL/Config.h>
#include "TokenList.h"
#include <array>
#include <float.h>

namespace msl
{

class Output;

class Parser
{
public:
	enum Options
	{
		RemoveUniformBlocks = 0x1
	};

	static const unsigned int maxAttachments = 16;

	static const std::uint32_t unsetInt = 0xFFFFFFFF;

	enum class Stage
	{
		Vertex,
		TessellationControl,
		TessellationEvaluation,
		Geometry,
		Fragment,
		Compute
	};

	static const unsigned int stageCount = static_cast<unsigned int>(Stage::Compute) + 1;

	enum class Bool
	{
		Unset = -1,
		False,
		True
	};

	enum class Filter
	{
		Unset = -1,
		Nearest,
		Linear
	};

	enum class PolygonMode
	{
		Unset = -1,
		Fill,
		Line,
		Point
	};

	enum class CullMode
	{
		Unset = -1,
		None,
		Front,
		Back,
		FrontAndBack
	};

	enum class FrontFace
	{
		Unset = -1,
		CounterClockwise,
		Clockwise
	};

	enum class StencilOp
	{
		Unset = -1,
		Keep,
		Zero,
		Replace,
		IncrementAndClamp,
		DecrementAndClamp,
		Invert,
		IncrementAndWrap,
		DecrementAndWrap
	};

	enum class CompareOp
	{
		Unset = -1,
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	enum class BlendFactor
	{
		Unset = -1,
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstColor,
		OneMinusConstColor,
		ConstAlpha,
		OneMinusConstAlpha,
		SrcAlphaSaturate,
		Src1Color,
		OneMinusSrc1Color,
		Src1Alpha,
		OneMinusSrc1Alpha
	};

	enum class BlendOp
	{
		Unset = -1,
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum ColorMask
	{
		ColorMaskUnset = -1,
		ColorMaskNone = 0,
		ColorMaskRed = 0x1,
		ColorMaskGreen = 0x2,
		ColorMaskBlue = 0x4,
		ColorMaskAlpha = 0x8
	};

	enum class LogicOp
	{
		Unset = -1,
		Clear,
		And,
		AndReverse,
		Copy,
		AndInverted,
		NoOp,
		Xor,
		Or,
		Nor,
		Equivalent,
		Invert,
		OrReverse,
		CopyInverted,
		OrInverted,
		Nand,
		Set
	};

	enum class MipFilter
	{
		Unset = -1,
		None,
		Nearest,
		Linear,
		Anisotropic
	};

	enum class AddressMode
	{
		Unset = -1,
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorOnce
	};

	enum class BorderColor
	{
		Unset = -1,
		TransparentBlack,
		OpaqueBlack,
		OpaqueWhite
	};

	struct RasterizationState
	{
		Bool depthClampEnable = Bool::Unset;
		Bool rasterizerDiscardEnable = Bool::Unset;
		PolygonMode polygonMode = PolygonMode::Unset;
		CullMode cullMode = CullMode::Unset;
		FrontFace frontFace = FrontFace::Unset;
		Bool depthBiasEnable = Bool::Unset;
		float depthBiasConstantFactor = FLT_MAX;
		float depthBiasClamp = FLT_MAX;
		float depthBiasSlopeFactor = FLT_MAX;
		float lineWidth = FLT_MAX;
	};

	struct MultisampleState
	{
		Bool sampleShadingEnable = Bool::Unset;
		float minSampleShading = FLT_MAX;
		std::uint32_t sampleMask = unsetInt;
		Bool alphaToCoverageEnable = Bool::Unset;
		Bool alphaToOneEnable = Bool::Unset;
	};

	struct StencilOpState
	{
		StencilOp failOp = StencilOp::Unset;
		StencilOp passOp = StencilOp::Unset;
		StencilOp depthFailOp = StencilOp::Unset;
		CompareOp compareOp = CompareOp::Unset;
		std::uint32_t compareMask = unsetInt;
		std::uint32_t writeMask = unsetInt;
		std::uint32_t reference = unsetInt;
	};

	struct DepthStencilState
	{
		Bool depthTestEnable = Bool::Unset;
		Bool depthWriteEnable = Bool::Unset;
		CompareOp depthCompareOp = CompareOp::Unset;
		Bool depthBoundsTestEnable = Bool::Unset;
		Bool stencilTestEnable = Bool::Unset;
		StencilOpState frontStencil;
		StencilOpState backStencil;
		float minDepthBounds = FLT_MAX;
		float maxDepthBounds = FLT_MAX;
	};

	struct BlendAttachmentState
	{
		Bool blendEnable = Bool::Unset;
		BlendFactor srcColorBlendFactor = BlendFactor::Unset;
		BlendFactor dstColorBlendFactor = BlendFactor::Unset;
		BlendOp colorBlendOp = BlendOp::Unset;
		BlendFactor srcAlphaBlendFactor = BlendFactor::Unset;
		BlendFactor dstAlphaBlendFactor = BlendFactor::Unset;
		BlendOp alphaBlendOp = BlendOp::Unset;
		ColorMask colorWriteMask = ColorMaskUnset;
	};

	struct BlendState
	{
		Bool logicalOpEnable = Bool::Unset;
		LogicOp logicalOp = LogicOp::Unset;
		Bool separateAttachmentBlending = Bool::Unset;
		std::array<BlendAttachmentState, maxAttachments> blendAttachments;
		std::array<float, 4> blendConstants = {{FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX}};
	};

	struct RenderState
	{
		RasterizationState rasterizationState;
		MultisampleState multisampleState;
		DepthStencilState depthStencilState;
		BlendState blendState;
		std::uint32_t patchControlPoints = unsetInt;
	};

	struct Pipeline
	{
		const Token* token;
		std::string name;
		std::array<std::string, stageCount> entryPoints;
		RenderState renderState;
	};

	struct Sampler
	{
		const Token* token;
		std::string name;
		Filter minFilter = Filter::Unset;
		Filter magFilter = Filter::Unset;
		MipFilter mipFilter = MipFilter::Unset;
		AddressMode addressModeU = AddressMode::Unset;
		AddressMode addressModeV = AddressMode::Unset;
		AddressMode addressModeW = AddressMode::Unset;
		float mipLodBias = FLT_MAX;
		Bool anisotropicFiltering = Bool::Unset;
		float maxAnisotropy = FLT_MAX;
		float minLod = FLT_MAX;
		float maxLod = FLT_MAX;
		BorderColor borderColor = BorderColor::Unset;
	};

	struct LineMapping
	{
		const char* fileName;
		std::size_t line;
	};

	Parser()
		: m_options(0)
	{
	}

	TokenList& getTokens()
	{
		return m_tokens;
	}

	const std::vector<Pipeline>& getPipelines() const
	{
		return m_pipelines;
	}

	bool parse(Output& output, int options = 0);
	std::string createShaderString(std::vector<LineMapping>& lineMappings, const Pipeline& pipeline,
		Stage stage) const;

private:
	enum class Element
	{
		Precision,
		Struct,
		FreeUniform,
		UniformBlock,
		Default
	};

	static const unsigned int elementCount = static_cast<unsigned int>(Element::Default) + 1;

	struct TokenRange
	{
		std::size_t start;
		std::size_t count;
	};

	Element getElementType(const TokenRange& tokenRange) const;
	void endElement(std::vector<Stage>& stages, TokenRange& tokenRange, std::size_t index);
	bool readPipeline(Output& output, const std::vector<Token>& tokens, std::size_t& i);
	void addElementString(std::string& str, std::vector<LineMapping>& lineMappings,
		const TokenRange& tokenRange, const std::string& entryPoint) const;
	bool removeUniformBlock(std::string& str, std::vector<LineMapping>& lineMappings,
		const TokenRange& tokenRange) const;

	TokenList m_tokens;

	int m_options;
	std::array<std::array<std::vector<TokenRange>, stageCount>, elementCount> m_elements;
	std::vector<Pipeline> m_pipelines;
};

} // namespace msl
