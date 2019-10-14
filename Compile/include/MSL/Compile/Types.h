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
#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

/**
 * @file
 * @brief Common types and constants used throughout the library.
 */

namespace msl
{

class Target;

namespace compile
{

/**
 * @brief Constant for no shader being set.
 */
static const std::size_t noShader = static_cast<std::size_t>(-1);

/**
 * @brief Constant for no known value.
 */
static const std::uint32_t unknown = static_cast<std::uint32_t>(-1);

/**
 * @brief Constant for no known float value.
 */
static const float unknownFloat = std::numeric_limits<float>::max();

/**
 * @brief Constant for the maximum number of color attachments for a pixel shader.
 */
static const unsigned int maxAttachments = 16;

/**
 * @brief Enum for a stage within a shader pipeline.
 */
enum class Stage
{
	Vertex,                 ///< Vertex shader.
	TessellationControl,    ///< Tessellation control for tessellation shaders.
	TessellationEvaluation, ///< Evaluation for tessellation shaders.
	Geometry,               ///< Geometry shader.
	Fragment,               ///< Fragment shader.
	Compute                 ///< Compute shader.
};

/**
 * @brief Constant for the number of pipeline stages.
 */
static const unsigned int stageCount = static_cast<unsigned int>(Stage::Compute) + 1;

/**
 * @brief Enum for the type of a uniform.
 */
enum class Type
{
	// Scalars and vectors
	Float,  ///< float
	Vec2,   ///< vec2
	Vec3,   ///< vec3
	Vec4,   ///< vec4
	Double, ///< double
	DVec2,  ///< dvec2
	DVec3,  ///< dvec3
	DVec4,  ///< dvec4
	Int,    ///< int
	IVec2,  ///< ivec2
	IVec3,  ///< ivec3
	IVec4,  ///< ivec4
	UInt,   ///< unsigned int
	UVec2,  ///< uvec2
	UVec3,  ///< uvec3
	UVec4,  ///< uvec4
	Bool,   ///< bool
	BVec2,  ///< bvec2
	BVec3,  ///< bvec3
	BVec4,  ///< bvec4

	// Matrices
	Mat2,    ///< mat2, mat2x2
	Mat3,    ///< mat3, mat3x3
	Mat4,    ///< mat4, mat4x4
	Mat2x3,  ///< mat2x3
	Mat2x4,  ///< mat2x4
	Mat3x2,  ///< mat3x2
	Mat3x4,  ///< mat3x4
	Mat4x2,  ///< mat4x2
	Mat4x3,  ///< mat4x3
	DMat2,   ///< dmat2, dmat2x2
	DMat3,   ///< dmat3, dmat3x3
	DMat4,   ///< dmat4, dmat4x4
	DMat2x3, ///< dmat2x3
	DMat2x4, ///< dmat2x4
	DMat3x2, ///< dmat3x2
	DMat3x4, ///< dmat3x4
	DMat4x2, ///< dmat4x2
	DMat4x3, ///< dmat4x3

	// Samplers
	Sampler1D,            ///< sampler1D
	Sampler2D,            ///< sampler2D
	Sampler3D,            ///< sampler3D
	SamplerCube,          ///< samplerCube
	Sampler1DShadow,      ///< sampler1DShadow
	Sampler2DShadow,      ///< sampler2DShadow
	Sampler1DArray,       ///< sampler1DArray
	Sampler2DArray,       ///< sampler2DArray
	Sampler1DArrayShadow, ///< sampler1DArrayShadow
	Sampler2DArrayShadow, ///< sampler2DArrayShadow
	Sampler2DMS,          ///< sampler2DMS
	Sampler2DMSArray,     ///< sampler2DMSArray
	SamplerCubeShadow,    ///< samplerCubeShadow
	SamplerBuffer,        ///< samplerBuffer
	Sampler2DRect,        ///< sampler2DRect
	Sampler2DRectShadow,  ///< sampler2DRectShadow
	ISampler1D,           ///< isampler1D
	ISampler2D,           ///< isampler2D
	ISampler3D,           ///< isampler3D
	ISamplerCube,         ///< isamplerCube
	ISampler1DArray,      ///< isampler1DArray
	ISampler2DArray,      ///< isampler2DArray
	ISampler2DMS,         ///< isampler2DMS
	ISampler2DMSArray,    ///< isampler2DMSArray
	ISampler2DRect,       ///< isampler2DRect
	USampler1D,           ///< usampler1D
	USampler2D,           ///< usampler2D
	USampler3D,           ///< usampler3D
	USamplerCube,         ///< usamplerCube
	USampler1DArray,      ///< usampler1DArray
	USampler2DArray,      ///< usampler2DArray
	USampler2DMS,         ///< usampler2DMS
	USampler2DMSArray,    ///< usampler2DMSArray
	USampler2DRect,       ///< usampler2DRect

	// Images
	Image1D,         ///< image1D
	Image2D,         ///< image2D
	Image3D,         ///< image3D
	ImageCube,       ///< imageCube
	Image1DArray,    ///< image1DArray
	Image2DArray,    ///< image2DArray
	Image2DMS,       ///< image2DMS
	Image2DMSArray,  ///< image2DMSArray
	ImageBuffer,     ///< imageBuffer
	Image2DRect,     ///< image2DRect
	IImage1D,        ///< iimage1D
	IImage2D,        ///< iimage2D
	IImage3D,        ///< iimage3D
	IImageCube,      ///< iimageCube
	IImage1DArray,   ///< iimage1DArray
	IImage2DArray,   ///< iimage2DArray
	IImage2DMS,      ///< iimage2DMS
	IImage2DMSArray, ///< iimage2DMSArray
	IImage2DRect,    ///< iimage2DRect
	UImage1D,        ///< uimage1D
	UImage2D,        ///< uimage2D
	UImage3D,        ///< uimage3D
	UImageCube,      ///< uimageCube
	UImage1DArray,   ///< uimage1DArray
	UImage2DArray,   ///< uimage2DArray
	UImage2DMS,      ///< uimage2DMS
	UImage2DMSArray, ///< uimage2DMSArray
	UImage2DRect,    ///< uimage2DRect

	// Subpass inputs.
	SubpassInput,    ///< subpassInput
	SubpassInputMS,  ///< subpassInputMS
	ISubpassInput,   ///< isubpassInput
	ISubpassInputMS, ///< isubpassInputMS
	USubpassInput,   ///< usubpassInput
	USubpassInputMS, ///< usubpassInputMS

	// Other.
	Struct, ///< User-defined structure.
};

/**
 * @brief Constant for the number of types.
 */
static const unsigned int typeCount = static_cast<unsigned int>(Type::Struct) + 1;

/**
 * @brief Enum for how a uniform is used.
 */
enum class UniformType
{
	/**
	 * Push-constant buffer. For non-Vulkan targets, the structure elements will become free
	 * uniforms. This will always be a struct.
	 */
	PushConstant,

	/**
	 * A uniform block, which cannot be written to from the shader. This will always be a struct.
	 */
	Block,

	/**
	 * A uniform block buffer, which can be written to from the shader. This will always be a
	 * struct.
	 */
	BlockBuffer,

	/**
	 * An image that doesn't use a sampler.
	 */
	Image,

	/**
	 * An image that uses a sampler to perform filtering, mipmapping, etc.
	 */
	SampledImage,

	/**
	 * Framebuffer input for subpasses.
	 */
	SubpassInput
};

/**
 * @brief Enum for a boolean value that may be unset.
 */
enum class Bool
{
	Unset = -1, ///< No value set.
	False,      ///< false
	True        ///< true
};

/**
 * @brief Enum for the polygon mode.
 */
enum class PolygonMode
{
	Unset = -1, ///< No value set.
	Fill,       ///< Draw filled polygons.
	Line,       ///< Draw outlines of polygons.
	Point       ///< Draw points for each vertex.
};

/**
 * @brief Enum for the cull mode.
 */
enum class CullMode
{
	Unset = -1,  ///< No value set.
	None,        ///< Don't cull any faces.
	Front,       ///< Cull front faces.
	Back,        ///< Cull back faces.
	FrontAndBack ///< Cull front and back faces.
};

/**
 * @brief Enum for the front face.
 */
enum class FrontFace
{
	Unset = -1,       ///< No value set.
	CounterClockwise, ///< Counter-clockwise faces are front.
	Clockwise         ///< Clockwise faces are front.
};

/**
 * @brief Enum for stencil operations.
 */
enum class StencilOp
{
	Unset = -1,        ///< No value set.
	Keep,              ///< Keep the current value.
	Zero,              ///< Set the value to 0.
	Replace,           ///< Replace the current value.
	IncrementAndClamp, ///< Increment the value, clamping to the maximum.
	DecrementAndClamp, ///< Decrement the value, clamping to 0.
	Invert,            ///< Inverts the bits of the value.
	IncrementAndWrap,  ///< Increments the value, wrapping around to 0.
	DecrementAndWrap   ///< Decrements the value, wrapping around to the maximum.
};

/**
 * @brief Enum for compare operations.
 */
enum class CompareOp
{
	Unset = -1,     ///< No value set.
	Never,          ///< Never succeed.
	Less,           ///< Check if a < b.
	Equal,          ///< Check if a == b.
	LessOrEqual,    ///< Check if a <= b.
	Greater,        ///< Check if a > b.
	NotEqual,       ///< Check if a != b.
	GreaterOrEqual, ///< Check if a >= b.
	Always          ///< Always succeed.
};

/**
 * @brief Enum for a blend factor.
 */
enum class BlendFactor
{
	Unset = -1,         ///< No value set.
	Zero,               ///< Value of 0.
	One,                ///< Value of 1.
	SrcColor,           ///< Source color. (from the current fragment)
	OneMinusSrcColor,   ///< 1 - source color.
	DstColor,           ///< Destination color. (from the framebuffer)
	OneMinusDstColor,   ///< 1 - destination color.
	SrcAlpha,           ///< Source alpha. (from the current fragment)
	OneMinusSrcAlpha,   ///< 1 - source alpha.
	DstAlpha,           ///< Destination alpha. (from the framebuffer)
	OneMinusDstAlpha,   ///< 1 - destination alpha.
	ConstColor,         ///< Constant user-specified color.
	OneMinusConstColor, ///< 1 - const color.
	ConstAlpha,         ///< Constant uer-specified alpha.
	OneMinusConstAlpha, ///< 1 - const alpha.
	SrcAlphaSaturate,   ///< Source alpha, clamped the range [0, 1].
	Src1Color,          ///< Source color from the secondary color buffer.
	OneMinusSrc1Color,  ///< 1 - secondary source color.
	Src1Alpha,          ///< Source alpha from the secondary color buffer.
	OneMinusSrc1Alpha   ///< 1 - secondary source alpha.
};

/**
 * @brief Enum for a blend operation.
 */
enum class BlendOp
{
	Unset = -1,      ///< No value set.
	Add,             ///< Evaluates a + b.
	Subtract,        ///< Evaluates a - b.
	ReverseSubtract, ///< Evaluates b - a.
	Min,             ///< Evaluates min(a, b).
	Max              ///< Evaluates max(a, b).
};

/**
 * @brief Enum for a color mask.
 *
 * These values can be OR'd together.
 */
enum ColorMask
{
	ColorMaskUnset =  -1, ///< No value set.
	ColorMaskNone =    0, ///< Write no color channels.
	ColorMaskRed =   0x1, ///< Write the red channel.
	ColorMaskGreen = 0x2, ///< Write the green channel.
	ColorMaskBlue =  0x4, ///< Write the blue channel.
	ColorMaskAlpha = 0x8  ///< Write the alpha channel.
};

/**
 * @brief Enum for a logical operation.
 */
enum class LogicOp
{
	Unset = -1,   ///< No value set.
	Clear,        ///< Clear the value to 0.
	And,          ///< Evaluate a & b.
	AndReverse,   ///< Evaluate a & ~b.
	Copy,         ///< Copy a to b.
	AndInverted,  ///< Evaluate ~a & b.
	NoOp,         ///< Don't modify the value.
	Xor,          ///< Evaluate a ^ b.
	Or,           ///< Evaluate a | b.
	Nor,          ///< Evaluate ~(a | b).
	Equivalent,   ///< Evaluate ~(a ^ b).
	Invert,       ///< Evaluate ~b.
	OrReverse,    ///< Evaluate a | ~b.
	CopyInverted, ///< Evaluate ~a.
	OrInverted,   ///< Evaluate ~a | b.
	Nand,         ///< Evaluate ~(a & b).
	Set           ///< Set the value to all 1.
};

/**
 * @brief Enum for how to filter a texture.
 */
enum class Filter
{
	Unset = -1, ///< No value set.
	Nearest,    ///< Nearest-neighbor filtering.
	Linear      ///< Linear filtering.
};

/**
 * @brief Enum for how to filter between mips.
 */
enum class MipFilter
{
	Unset = -1, ///< No value set.
	None,       ///< No mip-mapping.
	Nearest,    ///< Nearest-neighbor filtering.
	Linear,     ///< Linear filtering.
	Anisotropic ///< Anisotropic filtering.
};

/**
 * @brief Enum for how to handle texture addressing.
 */
enum class AddressMode
{
	Unset = -1,     ///< No value set.
	Repeat,         ///< Repeat the texture beyond the boundary.
	MirroredRepeat, ///< Repeat the textore, mirroring on each odd repeat.
	ClampToEdge,    ///< Clamp to the edge, using the texture value along the edge.
	ClampToBorder,  ///< Clamp to the edge, using the border color.
	MirrorOnce      ///< Mirror the texture once before clamping it.
};

/**
 * @brief Enum for the border color when using AddressMode ClampToBorder.
 */
enum class BorderColor
{
	Unset = -1,         ///< No value set.
	TransparentBlack,   ///< All color channels and alpha 0.
	TransparentIntZero, ///< All color channels and alpha 0. (as integers)
	OpaqueBlack,        ///< Color channels 0, alpha value 1.
	OpaqueIntZero,      ///< Color channels 0, alpha value as the int value 1.
	OpaqueWhite,        ///< All color channels and alpha 1.
	OpaqueIntOne        ///< All color channels and alpha as the int value 1.
};

/**
 * @brief Structure holding the render states used for rasterization.
 */
struct RasterizationState
{
	/**
	 * @brief Clamp the depth values in range rather than clipping.
	 */
	Bool depthClampEnable = Bool::Unset;

	/**
	 * @brief Whether or not to discard all samples.
	 */
	Bool rasterizerDiscardEnable = Bool::Unset;

	/**
	 * @brief Mode for how to draw polygons.
	 */
	PolygonMode polygonMode = PolygonMode::Unset;

	/**
	 * @brief Mode for how to cull front and back faces.
	 */
	CullMode cullMode = CullMode::Unset;

	/**
	 * @brief The polygon front face.
	 */
	FrontFace frontFace = FrontFace::Unset;

	/**
	 * @brief Whether or not to use depth bias.
	 */
	Bool depthBiasEnable = Bool::Unset;

	/**
	 * @brief The constant depth bias to apply.
	 */
	float depthBiasConstantFactor = unknownFloat;

	/**
	 * @brief The minimum or maximum value to clamp the depth bias to.
	 */
	float depthBiasClamp = unknownFloat;

	/**
	 * @brief The depth bias to apply based on the slope of the polygon.
	 */
	float depthBiasSlopeFactor = unknownFloat;

	/**
	 * @brief The width of lines.
	 */
	float lineWidth = unknownFloat;
};

/**
 * @brief Structure for holding multisampling render states.
 */
struct MultisampleState
{
	/**
	 * @brief Whether or not to run the shader for multiple samples.
	 */
	Bool sampleShadingEnable = Bool::Unset;

	/**
	 * @brief Hint for how many samples to run the shader on.
	 */
	float minSampleShading = unknownFloat;

	/**
	 * @brief Mask for which samples to run the shader on.
	 */
	std::uint32_t sampleMask = unknown;

	/**
	 * @brief Whether or not to use the alpha value to control how many samples to use.
	 */
	Bool alphaToCoverageEnable = Bool::Unset;

	/**
	 * @brief Whether or not to force the alpha value to 1.
	 */
	Bool alphaToOneEnable = Bool::Unset;
};

/**
 * @brief Structure for holding the stencil state.
 */
struct StencilOpState
{
	/**
	 * @brief The operation to perform when failing the stencil test.
	 */
	StencilOp failOp = StencilOp::Unset;

	/**
	 * @brief The operation to perform when passing the stencil test.
	 */
	StencilOp passOp = StencilOp::Unset;

	/**
	 * @brief The operation to perform when failing the depth test.
	 */
	StencilOp depthFailOp = StencilOp::Unset;

	/**
	 * @brief The compare operation for stencil values.
	 */
	CompareOp compareOp = CompareOp::Unset;

	/**
	 * @brief The mask to apply to the values for comparisson.
	 */
	std::uint32_t compareMask = unknown;

	/**
	 * @brief The mask to apply to the value before writing to the stencil buffer.
	 */
	std::uint32_t writeMask = unknown;

	/**
	 * @brief Constant reference value.
	 */
	std::uint32_t reference = unknown;
};

/**
 * @brief Structure for holding the depth render states.
 */
struct DepthStencilState
{
	/**
	 * @brief Whether or not to enable the depth test.
	 */
	Bool depthTestEnable = Bool::Unset;

	/**
	 * @brief Whether or not to write the depth value to the depth buffer.
	 */
	Bool depthWriteEnable = Bool::Unset;

	/**
	 * @brief The comparisson operation for depth values.
	 */
	CompareOp depthCompareOp = CompareOp::Unset;

	/**
	 * @brief Whether or not to limit the depth range.
	 */
	Bool depthBoundsTestEnable = Bool::Unset;

	/**
	 * @brief Whether or not to enable the stencil test.
	 */
	Bool stencilTestEnable = Bool::Unset;

	/**
	 * @brief Stencil operations for front faces.
	 */
	StencilOpState frontStencil;

	/**
	 * @brief Stencil operations for back faces.
	 */
	StencilOpState backStencil;

	/**
	 * @brief Minimum value when limiting the depth range.
	 */
	float minDepthBounds = unknownFloat;

	/**
	 * @brief Maximum value when limiting the depth range.
	 */
	float maxDepthBounds = unknownFloat;
};

/**
 * @brief Structure for holding the blend states for a color attachment.
 */
struct BlendAttachmentState
{
	/**
	 * @brief Whether or not to enable blending.
	 */
	Bool blendEnable = Bool::Unset;

	/**
	 * @brief Blend factor for the source color.
	 */
	BlendFactor srcColorBlendFactor = BlendFactor::Unset;

	/**
	 * @brief Blend factor for the destination color.
	 */
	BlendFactor dstColorBlendFactor = BlendFactor::Unset;

	/**
	 * @brief The operation to apply to the source and destination color factors.
	 */
	BlendOp colorBlendOp = BlendOp::Unset;

	/**
	 * @brief Blend factor for the source alpha.
	 */
	BlendFactor srcAlphaBlendFactor = BlendFactor::Unset;

	/**
	 * @brief Blend factor for the destination alpha.
	 */
	BlendFactor dstAlphaBlendFactor = BlendFactor::Unset;

	/**
	 * @brief The operation to apply to the source and destination alpha factors.
	 */
	BlendOp alphaBlendOp = BlendOp::Unset;

	/**
	 * @brief Mask of color channels to write to.
	 */
	ColorMask colorWriteMask = ColorMaskUnset;
};

/**
 * @brief Structure for holding the blend states.
 */
struct BlendState
{
	/**
	 * @brief Whether or not to enable logical operations.
	 */
	Bool logicalOpEnable = Bool::Unset;

	/**
	 * @brief The logical operation to apply.
	 */
	LogicOp logicalOp = LogicOp::Unset;

	/**
	 * @brief Whether or not to apply separate blending operations for each attachment.
	 *
	 * If unset, only the first attachment blend states should be used.
	 */
	Bool separateAttachmentBlendingEnable = Bool::Unset;

	/**
	 * @brief The blend states for each color attachment.
	 */
	std::array<BlendAttachmentState, maxAttachments> blendAttachments;

	/**
	 * @brief The constant blend factor.
	 */
	std::array<float, 4> blendConstants = {{unknownFloat, unknownFloat, unknownFloat, unknownFloat}};
};

/**
 * @brief Structure for holding the render states.
 */
struct RenderState
{
	/**
	 * @brief The rasterization states.
	 */
	RasterizationState rasterizationState;

	/**
	 * @brief The multisample states.
	 */
	MultisampleState multisampleState;

	/**
	 * @brief The depth-stencil states.
	 */
	DepthStencilState depthStencilState;

	/**
	 * @brief The blending states.
	 */
	BlendState blendState;

	/**
	 * @brief The number of control points for each patch for tessellation.
	 */
	std::uint32_t patchControlPoints = unknown;

	/**
	 * @brief The number of clip distances.
	 */
	std::uint32_t clipDistanceCount = 0;

	/**
	 * @brief The number of cull distances.
	 */
	std::uint32_t cullDistanceCount = 0;

	/**
	 * @brief Whether or not to enable early fragment tests.
	 */
	Bool earlyFragmentTests = Bool::Unset;
};

/**
 * @brief Structure for holding the states used for a texture sampler.
 */
struct SamplerState
{
	/**
	 * @brief The filter used for minification.
	 */
	Filter minFilter = Filter::Unset;

	/**
	 * @brief The filter used for magnification.
	 */
	Filter magFilter = Filter::Unset;

	/**
	 * @brief The filter used for mip-mapping.
	 */
	MipFilter mipFilter = MipFilter::Unset;

	/**
	 * @brief How to address the U (or S) texture coordinate.
	 */
	AddressMode addressModeU = AddressMode::Unset;

	/**
	 * @brief How to address the U (or T) texture coordinate.
	 */
	AddressMode addressModeV = AddressMode::Unset;

	/**
	 * @brief How to address the W (or R) texture coordinate.
	 */
	AddressMode addressModeW = AddressMode::Unset;

	/**
	 * @brief Bias to apply when calculating the mip-mapping LOD.
	 */
	float mipLodBias = unknownFloat;

	/**
	 * @brief The maximum anisotropy factor to apply.
	 */
	float maxAnisotropy = unknownFloat;

	/**
	 * @brief The minimum mip level to use.
	 */
	float minLod = unknownFloat;

	/**
	 * @brief The maximum mip level to use.
	 */
	float maxLod = unknownFloat;

	/**
	 * @brief The border color to apply.
	 */
	BorderColor borderColor = BorderColor::Unset;

	/**
	 * @brief The compare op for shadow samplers.
	 */
	CompareOp compareOp = CompareOp::Unset;
};

/**
 * @brief Structure holding info for an array within a uniform or attribute.
 */
struct ArrayInfo
{
	/**
	 * @brief The length of the array.
	 *
	 * This will be set to unknown if the size is unknown at compile time.
	 */
	std::uint32_t length;

	/**
	 * @brief The stride of the array.
	 */
	std::uint32_t stride;
};

/**
 * @brief Structure describing a member of a user-defined struct
 */
struct StructMember
{
	/**
	 * @brief The name of the element.
	 */
	std::string name;

	/**
	 * @brief The offset from the beginning of the structure.
	 */
	std::uint32_t offset;

	/**
	 * @brief The size of this member.
	 */
	std::uint32_t size;

	/**
	 * @brief The type of the element.
	 */
	Type type;

	/**
	 * @brief If type is Struct, this is the index to the stucture the member refers to.
	 */
	std::uint32_t structIndex;

	/**
	 * @brief Info about each element in the array.
	 *
	 * This will be empty if this member isn't an array.
	 */
	std::vector<ArrayInfo> arrayElements;

	/**
	 * @brief True if type is a matrix type and is row major.
	 */
	bool rowMajor;
};

/**
 * @brief Structure describing a user-defined struct.
 */
struct Struct
{
	/**
	 * @brief The type name of the struct.
	 */
	std::string name;

	/**
	 * @brief The size of the structure.
	 *
	 * If an element contains an unknown array length, this will be the size of the known
	 * elements.
	 */
	std::uint32_t size;

	/**
	 * @brief The members of the structures.
	 */
	std::vector<StructMember> members;
};

/**
 * @brief Structure defining a uniform.
 */
struct Uniform
{
	/**
	 * @brief The name of the uniform.
	 *
	 * If the uniform is a structure, it is the same as the structure name.
	 */
	std::string name;

	/**
	 * @brief The uniform type, describing how the uniform is bound.
	 */
	UniformType uniformType;

	/**
	 * @brief The type of the uniform value.
	 */
	Type type;

	/**
	 * @brief If type is Struct, this is the index to the stucture the member refers to.
	 */
	std::uint32_t structIndex;

	/**
	 * @brief Info about each element in the array.
	 *
	 * This will be empty if this uniform isn't an array.
	 */
	std::vector<ArrayInfo> arrayElements;

	/**
	 * @brief The index of the descriptor set.
	 *
	 * This may be set to unknown if not set.
	 */
	std::uint32_t descriptorSet;

	/**
	 * @brief The binding index.
	 *
	 * This may be set to unknown if not set.
	 */
	std::uint32_t binding;

	/**
	 * @brief The input attachment index for subpass inputs.
	 *
	 * This may be set to unknown if not set.
	 */
	std::uint32_t inputAttachmentIndex;

	/**
	 * @brief The index of the sampler state to use for sampler states.
	 *
	 * This will be set to unknown for non-sampler types or when no sampler state is defined.
	 */
	std::uint32_t samplerIndex;
};

/**
 * @brief Structure describing a vertex input attribute.
 */
struct Attribute
{
	/**
	 * @brief The name of the attribute.
	 */
	std::string name;

	/**
	 * @brief The type of the attribute.
	 */
	Type type;

	/**
	 * @brief The length of each array.
	 *
	 * This will be empty if this attribute isn't an array.
	 */
	std::vector<std::uint32_t> arrayElements;

	/**
	 * @brief The location of the attribute.
	 */
	std::uint32_t location;

	/**
	 * @brief The first component at location to use.
	 */
	std::uint32_t component;
};

/**
 * @brief Structure describing a fragment color output.
 */
struct FragmentOutput
{
	/**
	 * @brief The name of the output.
	 */
	std::string name;

	/**
	 * @brief The location of the output.
	 */
	std::uint32_t location;
};

/**
 * @brief Structure defining a shader within the pipeline.
 */
struct Shader
{
	/**
	 * @brief The index of the shader data.
	 *
	 * This is set to noShader if this stage isn't used.
	 */
	std::size_t shader;

	/**
	 * @brief The IDs for each unfirom.
	 *
	 * Each unused uniform will be set to unknown, and the vector will be empty if shader is set
	 * to noShader. This will correspond to the SPIR-V IDs for the uniforms, which can be used
	 * to easily adjust the descriptor sets or binding points of the uniforms with minimal
	 * processing.
	 */
	std::vector<std::uint32_t> uniformIds;
};

/**
 * @brief Structure that holds the information about the a pipeline within the compiled result.
 */
struct Pipeline
{
	/**
	 * @brief The user-defined structs declared for the pipeline.
	 */
	std::vector<Struct> structs;

	/**
	 * @brief The sampler states to apply to samplers.
	 */
	std::vector<SamplerState> samplerStates;

	/**
	 * @brief The uniforms declared for the pipeline.
	 */
	std::vector<Uniform> uniforms;

	/**
	 * @brief The vertex attributes used within the pipeline.
	 */
	std::vector<Attribute> attributes;

	/**
	 * @brief The fragment outputs used within the pipeline.
	 */
	std::vector<FragmentOutput> fragmentOutputs;

	/**
	 * @brief Index for the push constant structure.
	 *
	 * The push constant struct members will become individual uniforms for non-Vulkan targets.
	 * This is set to unknown if there are no push constants.
	 */
	std::uint32_t pushConstantStruct;

	/**
	 * @brief The render state to apply to the pipeline.
	 */
	RenderState renderState;

	/**
	 * @brief The local size for the compute stage, if present.
	 */
	std::array<std::uint32_t, 3> computeLocalSize = {{1, 1, 1}};

	/**
	 * @brief The shaders for the different pipeline stages.
	 *
	 * This can be indexed by the Stage enum.
	 */
	std::array<Shader, stageCount> shaders;

private:
	friend class msl::Target;

	// Internal use for error reporting.
	std::string file;
	std::size_t line = 0;
	std::size_t column = 0;
};

} // namespace compile
} // namespace msl
