/*
 * Copyright 2016-2019 Aaron Barany
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
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file
 * @brief Common types and constants used for the C interface.
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Constant for the current module file version.
 */
#define MSL_MODULE_VERSION 0

/**
 * @brief Constant for no known value.
 */
#define MSL_UNKNOWN (uint32_t)-1

/**
 * @brief Constant for no known float value.
 */
#define MSL_UNKNOWN_FLOAT FLT_MAX

/**
 * @brief Constant for the maximum number of color attachments for a pixel shader.
 */
#define MSL_MAX_ATTACHMENTS 16

/**
 * @brief Constant for the width of a texture buffer in Metal.
 */
#define MSL_METAL_TEXTURE_BUFFER_WIDTH 4096

/**
 * @brief Constant for the index of the push constant buffer in Metal.
 */
#define MSL_METAL_PUSH_CONSTANT_INDEX 0

/**
 * @brief Constant for the index of the tessellation indirect parameters buffer in Metal.
 *
 * This is used when translating tessellation shaders to Metal, which require manual separation of
 * vertex processing and tessellation stages.
 */
#define MSL_METAL_TESS_INDIRECT_PARAMS_INDEX 29

/**
 * @brief Constant for the index of vertex output buffer in Metal.
 *
 * This is used when translating tessellation shaders to Metal, which require manual separation of
 * vertex processing and tessellation stages.
 */
#define MSL_METAL_VERTEX_OUTPUT_INDEX 28

/**
 * @brief Constant for the index of the tessellation patch output buffer in Metal.
 *
 * This is used when translating tessellation shaders to Metal, which require manual separation of
 * vertex processing and tessellation stages.
 */
#define MSL_METAL_PATCH_OUTPUT_BUFFER_INDEX 27

/**
 * @brief Constant for the index of the tessellation factor output buffer in Metal.
 *
 * This is used when translating tessellation shaders to Metal, which require manual separation of
 * vertex processing and tessellation stages.
 */
#define MSL_METAL_TESS_FACTOR_OUTPUT_BUFFER_INDEX 26

/**
 * @brief Enum for a stage within a shader pipeline.
 */
typedef enum mslStage
{
	mslStage_Vertex,                 ///< Vertex shader.
	mslStage_TessellationControl,    ///< Tessellation control for tessellation shaders.
	mslStage_TessellationEvaluation, ///< Evaluation for tessellation shaders.
	mslStage_Geometry,               ///< Geometry shader.
	mslStage_Fragment,               ///< Fragment shader.
	mslStage_Compute,                ///< Compute shader.
	mslStage_Count                   ///< The number of stages.
} mslStage;

/**
 * @brief Enum for the type of a uniform.
 */
typedef enum mslType
{
	// Scalars and vectors
	mslType_Float,  ///< float
	mslType_Vec2,   ///< vec2
	mslType_Vec3,   ///< vec3
	mslType_Vec4,   ///< vec4
	mslType_Double, ///< double
	mslType_DVec2,  ///< dvec2
	mslType_DVec3,  ///< dvec3
	mslType_DVec4,  ///< dvec4
	mslType_Int,    ///< int
	mslType_IVec2,  ///< ivec2
	mslType_IVec3,  ///< ivec3
	mslType_IVec4,  ///< ivec4
	mslType_UInt,   ///< unsigned int
	mslType_UVec2,  ///< uvec2
	mslType_UVec3,  ///< uvec3
	mslType_UVec4,  ///< uvec4
	mslType_Bool,   ///< bool
	mslType_BVec2,  ///< bvec2
	mslType_BVec3,  ///< bvec3
	mslType_BVec4,  ///< bvec4

	// Matrices
	mslType_Mat2,    ///< mat2, mat2x2
	mslType_Mat3,    ///< mat3, mat3x3
	mslType_Mat4,    ///< mat4, mat4x4
	mslType_Mat2x3,  ///< mat2x3
	mslType_Mat2x4,  ///< mat2x4
	mslType_Mat3x2,  ///< mat3x2
	mslType_Mat3x4,  ///< mat3x4
	mslType_Mat4x2,  ///< mat4x2
	mslType_Mat4x3,  ///< mat4x3
	mslType_DMat2,   ///< dmat2, dmat2x2
	mslType_DMat3,   ///< dmat3, dmat3x3
	mslType_DMat4,   ///< dmat4, dmat4x4
	mslType_DMat2x3, ///< dmat2x3
	mslType_DMat2x4, ///< dmat2x4
	mslType_DMat3x2, ///< dmat3x2
	mslType_DMat3x4, ///< dmat3x4
	mslType_DMat4x2, ///< dmat4x2
	mslType_DMat4x3, ///< dmat4x3

	// Samplers
	mslType_Sampler1D,            ///< sampler1D
	mslType_Sampler2D,            ///< sampler2D
	mslType_Sampler3D,            ///< sampler3D
	mslType_SamplerCube,          ///< samplerCube
	mslType_Sampler1DShadow,      ///< sampler1DShadow
	mslType_Sampler2DShadow,      ///< sampler2DShadow
	mslType_Sampler1DArray,       ///< sampler1DArray
	mslType_Sampler2DArray,       ///< sampler2DArray
	mslType_Sampler1DArrayShadow, ///< sampler1DArrayShadow
	mslType_Sampler2DArrayShadow, ///< sampler2DArrayShadow
	mslType_Sampler2DMS,          ///< sampler2DMS
	mslType_Sampler2DMSArray,     ///< sampler2DMSArray
	mslType_SamplerCubeShadow,    ///< samplerCubeShadow
	mslType_SamplerBuffer,        ///< samplerBuffer
	mslType_Sampler2DRect,        ///< sampler2DRect
	mslType_Sampler2DRectShadow,  ///< sampler2DRectShadow
	mslType_ISampler1D,           ///< isampler1D
	mslType_ISampler2D,           ///< isampler2D
	mslType_ISampler3D,           ///< isampler3D
	mslType_ISamplerCube,         ///< isamplerCube
	mslType_ISampler1DArray,      ///< isampler1DArray
	mslType_ISampler2DArray,      ///< isampler2DArray
	mslType_ISampler2DMS,         ///< isampler2DMS
	mslType_ISampler2DMSArray,    ///< isampler2DMSArray
	mslType_ISampler2DRect,       ///< isampler2DRect
	mslType_USampler1D,           ///< usampler1D
	mslType_USampler2D,           ///< usampler2D
	mslType_USampler3D,           ///< usampler3D
	mslType_USamplerCube,         ///< usamplerCube
	mslType_USampler1DArray,      ///< usampler1DArray
	mslType_USampler2DArray,      ///< usampler2DArray
	mslType_USampler2DMS,         ///< usampler2DMS
	mslType_USampler2DMSArray,    ///< usampler2DMSArray
	mslType_USampler2DRect,       ///< usampler2DRect

	// Images
	mslType_Image1D,         ///< image1D
	mslType_Image2D,         ///< image2D
	mslType_Image3D,         ///< image3D
	mslType_ImageCube,       ///< imageCube
	mslType_Image1DArray,    ///< image1DArray
	mslType_Image2DArray,    ///< image2DArray
	mslType_Image2DMS,       ///< image2DMS
	mslType_Image2DMSArray,  ///< image2DMSArray
	mslType_ImageBuffer,     ///< imageBuffer
	mslType_Image2DRect,     ///< image2DRect
	mslType_IImage1D,        ///< iimage1D
	mslType_IImage2D,        ///< iimage2D
	mslType_IImage3D,        ///< iimage3D
	mslType_IImageCube,      ///< iimageCube
	mslType_IImage1DArray,   ///< iimage1DArray
	mslType_IImage2DArray,   ///< iimage2DArray
	mslType_IImage2DMS,      ///< iimage2DMS
	mslType_IImage2DMSArray, ///< iimage2DMSArray
	mslType_IImage2DRect,    ///< iimage2DRect
	mslType_UImage1D,        ///< uimage1D
	mslType_UImage2D,        ///< uimage2D
	mslType_UImage3D,        ///< uimage3D
	mslType_UImageCube,      ///< uimageCube
	mslType_UImage1DArray,   ///< uimage1DArray
	mslType_UImage2DArray,   ///< uimage2DArray
	mslType_UImage2DMS,      ///< uimage2DMS
	mslType_UImage2DMSArray, ///< uimage2DMSArray
	mslType_UImage2DRect,    ///< uimage2DRect

	// Subpass inputs.
	mslType_SubpassInput,    ///< subpassInput
	mslType_SubpassInputMS,  ///< subpassInputMS
	mslType_ISubpassInput,   ///< isubpassInput
	mslType_ISubpassInputMS, ///< isubpassInputMS
	mslType_USubpassInput,   ///< usubpassInput
	mslType_USubpassInputMS, ///< usubpassInputMS

	// Other.
	mslType_Struct, ///< User-defined structure.

	mslType_Count ///< The number of types.
} mslType;

/**
 * @brief Enum for how a uniform is used.
 */
typedef enum mslUniformType
{
	/**
	 * Push-constant buffer. For non-Vulkan targets, the structure elements will become free
	 * uniforms. This will always be a struct.
	 */
	mslUniformType_PushConstant,

	/**
	 * A uniform block, which cannot be written to from the shader. This will always be a struct.
	 */
	mslUniformType_Block,

	/**
	 * A uniform block buffer, which can be written to from the shader. This will always be a
	 * struct.
	 */
	mslUniformType_BlockBuffer,

	/**
	 * An image that doesn't use a sampler.
	 */
	mslUniformType_Image,

	/**
	 * An image that uses a sampler to perform filtering, mipmapping, etc.
	 */
	mslUniformType_SampledImage,

	/**
	 * Framebuffer input for subpasses.
	 */
	mslUniformType_SubpassInput
} mslUniformType;

/**
 * @brief Enum for a boolean value that may be unset.
 */
typedef enum mslBool
{
	mslBool_Unset = -1, ///< No value set.
	mslBool_False,      ///< false
	mslBool_True        ///< true
} mslBool;

/**
 * @brief Enum for the polygon mode.
 */
typedef enum mslPolygonMode
{
	mslPolygonMode_Unset = -1, ///< No value set.
	mslPolygonMode_Fill,       ///< Draw filled polygons.
	mslPolygonMode_Line,       ///< Draw outlines of polygons.
	mslPolygonMode_Point       ///< Draw points for each vertex.
} mslPolygonMode;

/**
 * @brief Enum for the cull mode.
 */
typedef enum mslCullMode
{
	mslCullMode_Unset = -1,  ///< No value set.
	mslCullMode_None,        ///< Don't cull any faces.
	mslCullMode_Front,       ///< Cull front faces.
	mslCullMode_Back,        ///< Cull back faces.
	mslCullMode_FrontAndBack ///< Cull front and back faces.
} mslCullMode;

/**
 * @brief Enum for the front face.
 */
typedef enum mslFrontFace
{
	mslFrontFace_Unset = -1,       ///< No value set.
	mslFrontFace_CounterClockwise, ///< Counter-clockwise faces are front.
	mslFrontFace_Clockwise         ///< Clockwise faces are front.
} mslFrontFace;

/**
 * @brief Enum for stencil operations.
 */
typedef enum mslStencilOp
{
	mslStencilOp_Unset = -1,        ///< No value set.
	mslStencilOp_Keep,              ///< Keep the current value.
	mslStencilOp_Zero,              ///< Set the value to 0.
	mslStencilOp_Replace,           ///< Replace the current value.
	mslStencilOp_IncrementAndClamp, ///< Increment the value, clamping to the maximum.
	mslStencilOp_DecrementAndClamp, ///< Decrement the value, clamping to 0.
	mslStencilOp_Invert,            ///< Inverts the bits of the value.
	mslStencilOp_IncrementAndWrap,  ///< Increments the value, wrapping around to 0.
	mslStencilOp_DecrementAndWrap   ///< Decrements the value, wrapping around to the maximum.
} mslStencilOp;

/**
 * @brief Enum for compare operations.
 */
typedef enum mslCompareOp
{
	mslCompareOp_Unset = -1,     ///< No value set.
	mslCompareOp_Never,          ///< Never succeed.
	mslCompareOp_Less,           ///< Check if a < b.
	mslCompareOp_Equal,          ///< Check if a == b.
	mslCompareOp_LessOrEqual,    ///< Check if a <= b.
	mslCompareOp_Greater,        ///< Check if a > b.
	mslCompareOp_NotEqual,       ///< Check if a != b.
	mslCompareOp_GreaterOrEqual, ///< Check if a >= b.
	mslCompareOp_Always          ///< Always succeed.
} mslCompareOp;

/**
 * @brief Enum for a blend factor.
 */
typedef enum mslBlendFactor
{
	mslBlendFactor_Unset = -1,         ///< No value set.
	mslBlendFactor_Zero,               ///< Value of 0.
	mslBlendFactor_One,                ///< Value of 1.
	mslBlendFactor_SrcColor,           ///< Source color. (from the current fragment)
	mslBlendFactor_OneMinusSrcColor,   ///< 1 - source color.
	mslBlendFactor_DstColor,           ///< Destination color. (from the framebuffer)
	mslBlendFactor_OneMinusDstColor,   ///< 1 - destination color.
	mslBlendFactor_SrcAlpha,           ///< Source alpha. (from the current fragment)
	mslBlendFactor_OneMinusSrcAlpha,   ///< 1 - source alpha.
	mslBlendFactor_DstAlpha,           ///< Destination alpha. (from the framebuffer)
	mslBlendFactor_OneMinusDstAlpha,   ///< 1 - destination alpha.
	mslBlendFactor_ConstColor,         ///< Constant user-specified color.
	mslBlendFactor_OneMinusConstColor, ///< 1 - const color.
	mslBlendFactor_ConstAlpha,         ///< Constant uer-specified alpha.
	mslBlendFactor_OneMinusConstAlpha, ///< 1 - const alpha.
	mslBlendFactor_SrcAlphaSaturate,   ///< Source alpha, clamped the range [0, 1].
	mslBlendFactor_Src1Color,          ///< Source color from the secondary color buffer.
	mslBlendFactor_OneMinusSrc1Color,  ///< 1 - secondary source color.
	mslBlendFactor_Src1Alpha,          ///< Source alpha from the secondary color buffer.
	mslBlendFactor_OneMinusSrc1Alpha   ///< 1 - secondary source alpha.
} mslBlendFactor;

/**
 * @brief Enum for a blend operation.
 */
typedef enum mslBlendOp
{
	mslBlendOp_Unset = -1,      ///< No value set.
	mslBlendOp_Add,             ///< Evaluates a + b.
	mslBlendOp_Subtract,        ///< Evaluates a - b.
	mslBlendOp_ReverseSubtract, ///< Evaluates b - a.
	mslBlendOp_Min,             ///< Evaluates min(a, b).
	mslBlendOp_Max              ///< Evaluates max(a, b).
} mslBlendOp;

/**
 * @brief Enum for a color mask.
 *
 * These values can be OR'd together.
 */
typedef enum mslColorMask
{
	mslColorMask_Unset =  -1, ///< No value set.
	mslColorMask_None =    0, ///< Write no color channels.
	mslColorMask_Red =   0x1, ///< Write the red channel.
	mslColorMask_Green = 0x2, ///< Write the green channel.
	mslColorMask_Blue =  0x4, ///< Write the blue channel.
	mslColorMask_Alpha = 0x8  ///< Write the alpha channel.
} mslColorMask;

/**
 * @brief Enum for a logical operation.
 */
typedef enum mslLogicOp
{
	mslLogicOp_Unset = -1,   ///< No value set.
	mslLogicOp_Clear,        ///< Clear the value to 0.
	mslLogicOp_And,          ///< Evaluate a & b.
	mslLogicOp_AndReverse,   ///< Evaluate a & ~b.
	mslLogicOp_Copy,         ///< Copy a to b.
	mslLogicOp_AndInverted,  ///< Evaluate ~a & b.
	mslLogicOp_NoOp,         ///< Don't modify the value.
	mslLogicOp_Xor,          ///< Evaluate a ^ b.
	mslLogicOp_Or,           ///< Evaluate a | b.
	mslLogicOp_Nor,          ///< Evaluate ~(a | b).
	mslLogicOp_Equivalent,   ///< Evaluate ~(a ^ b).
	mslLogicOp_Invert,       ///< Evaluate ~b.
	mslLogicOp_OrReverse,    ///< Evaluate a | ~b.
	mslLogicOp_CopyInverted, ///< Evaluate ~a.
	mslLogicOp_OrInverted,   ///< Evaluate ~a | b.
	mslLogicOp_Nand,         ///< Evaluate ~(a & b).
	mslLogicOp_Set           ///< Set the value to all 1.
} mslLogicOp;

/**
 * @brief Enum for how to filter a texture.
 */
typedef enum mslFilter
{
	mslFilter_Unset = -1, ///< No value set.
	mslFilter_Nearest,    ///< Nearest-neighbor filtering.
	mslFilter_Linear      ///< Linear filtering.
} mslFilter;

/**
 * @brief Enum for how to filter between mips.
 */
typedef enum mslMipFilter
{
	mslMipFilter_Unset = -1, ///< No value set.
	mslMipFilter_None,       ///< No mip-mapping.
	mslMipFilter_Nearest,    ///< Nearest-neighbor filtering.
	mslMipFilter_Linear,     ///< Linear filtering.
	mslMipFilter_Anisotropic ///< Anisotropic filtering.
} mslMipFilter;

/**
 * @brief Enum for how to handle texture addressing.
 */
typedef enum mslAddressMode
{
	mslAddressMode_Unset = -1,     ///< No value set.
	mslAddressMode_Repeat,         ///< Repeat the texture beyond the boundary.
	mslAddressMode_MirroredRepeat, ///< Repeat the textore, mirroring on each odd repeat.
	mslAddressMode_ClampToEdge,    ///< Clamp to the edge, using the texture value along the edge.
	mslAddressMode_ClampToBorder,  ///< Clamp to the edge, using the border color.
	mslAddressMode_MirrorOnce      ///< Mirror the texture once before clamping it.
} mslAddressMode;

/**
 * @brief Enum for the border color when using AddressMode ClampToBorder.
 */
typedef enum mslBorderColor
{
	mslBorderColor_Unset = -1,         ///< No value set.
	mslBorderColor_TransparentBlack,   ///< All color channels and alpha 0.
	mslBorderColor_TransparentIntZero, ///< All color channels and alpha 0. (as integers)
	mslBorderColor_OpaqueBlack,        ///< Color channels 0, alpha value 1.
	mslBorderColor_OpaqueIntZero,      ///< Color channels 0, alpha value as the int value 1.
	mslBorderColor_OpaqueWhite,        ///< All color channelsa and alpha 1.
	mslBorderColor_OpaqueIntOne        ///< All color channels and alpha as the int value 1.
} mslBorderColor;

/**
 * @brief Structure holding the render states used for rasterization.
 */
typedef struct mslRasterizationState
{
	/**
	 * @brief Clamp the depth values in range rather than clipping.
	 */
	mslBool depthClampEnable;

	/**
	 * @brief Whether or not to discard all samples.
	 */
	mslBool rasterizerDiscardEnable;

	/**
	 * @brief Mode for how to draw polygons.
	 */
	mslPolygonMode polygonMode;

	/**
	 * @brief Mode for how to cull front and back faces.
	 */
	mslCullMode cullMode;

	/**
	 * @brief The polygon front face.
	 */
	mslFrontFace frontFace;

	/**
	 * @brief Whether or not to use depth bias.
	 */
	mslBool depthBiasEnable;

	/**
	 * @brief The constant depth bias to apply.
	 */
	float depthBiasConstantFactor;

	/**
	 * @brief The minimum or maximum value to clamp the depth bias to.
	 */
	float depthBiasClamp;

	/**
	 * @brief The depth bias to apply based on the slope of the polygon.
	 */
	float depthBiasSlopeFactor;

	/**
	 * @brief The width of lines.
	 */
	float lineWidth;
} mslRasterizationState;

/**
 * @brief Structure for holding multisampling render states.
 */
typedef struct mslMultisampleState
{
	/**
	 * @brief Whether or not to run the shader for multiple samples.
	 */
	mslBool sampleShadingEnable;

	/**
	 * @brief Hint for how many samples to run the shader on.
	 */
	float minSampleShading;

	/**
	 * @brief Mask for which samples to run the shader on.
	 */
	uint32_t sampleMask;

	/**
	 * @brief Whether or not to use the alpha value to control how many samples to use.
	 */
	mslBool alphaToCoverageEnable;

	/**
	 * @brief Whether or not to force the alpha value to 1.
	 */
	mslBool alphaToOneEnable;
} mslMultisampleState;

/**
 * @brief Structure for holding the stencil state.
 */
typedef struct mslStencilOpState
{
	/**
	 * @brief The operation to perform when failing the stencil test.
	 */
	mslStencilOp failOp;

	/**
	 * @brief The operation to perform when passing the stencil test.
	 */
	mslStencilOp passOp;

	/**
	 * @brief The operation to perform when failing the depth test.
	 */
	mslStencilOp depthFailOp;

	/**
	 * @brief The compare operation for stencil values.
	 */
	mslCompareOp compareOp;

	/**
	 * @brief The mask to apply to the values for comparisson.
	 */
	uint32_t compareMask;

	/**
	 * @brief The mask to apply to the value before writing to the stencil buffer.
	 */
	uint32_t writeMask;

	/**
	 * @brief Constant reference value.
	 */
	uint32_t reference;
} mslStencilOpState;

/**
 * @brief Structure for holding the depth render states.
 */
typedef struct mslDepthStencilState
{
	/**
	 * @brief Whether or not to enable the depth test.
	 */
	mslBool depthTestEnable;

	/**
	 * @brief Whether or not to write the depth value to the depth buffer.
	 */
	mslBool depthWriteEnable;

	/**
	 * @brief The comparisson operation for depth values.
	 */
	mslCompareOp depthCompareOp;

	/**
	 * @brief Whether or not to limit the depth range.
	 */
	mslBool depthBoundsTestEnable;

	/**
	 * @brief Whether or not to enable the stencil test.
	 */
	mslBool stencilTestEnable;

	/**
	 * @brief Stencil operations for front faces.
	 */
	mslStencilOpState frontStencil;

	/**
	 * @brief Stencil operations for back faces.
	 */
	mslStencilOpState backStencil;

	/**
	 * @brief Minimum value when limiting the depth range.
	 */
	float minDepthBounds;

	/**
	 * @brief Maximum value when limiting the depth range.
	 */
	float maxDepthBounds;
} mslDepthStencilState;

/**
 * @brief Structure for holding the blend states for a color attachment.
 */
typedef struct mslBlendAttachmentState
{
	/**
	 * @brief Whether or not to enable blending.
	 */
	mslBool blendEnable;

	/**
	 * @brief Blend factor for the source color.
	 */
	mslBlendFactor srcColorBlendFactor;

	/**
	 * @brief Blend factor for the destination color.
	 */
	mslBlendFactor dstColorBlendFactor;

	/**
	 * @brief The operation to apply to the source and destination color factors.
	 */
	mslBlendOp colorBlendOp;

	/**
	 * @brief Blend factor for the source alpha.
	 */
	mslBlendFactor srcAlphaBlendFactor;

	/**
	 * @brief Blend factor for the destination alpha.
	 */
	mslBlendFactor dstAlphaBlendFactor;

	/**
	 * @brief The operation to apply to the source and destination alpha factors.
	 */
	mslBlendOp alphaBlendOp;

	/**
	 * @brief Mask of color channels to write to.
	 */
	mslColorMask colorWriteMask;
} mslBlendAttachmentState;

/**
 * @brief Structure for holding the blend states.
 */
typedef struct mslBlendState
{
	/**
	 * @brief Whether or not to enable logical operations.
	 */
	mslBool logicalOpEnable;

	/**
	 * @brief The logical operation to apply.
	 */
	mslLogicOp logicalOp;

	/**
	 * @brief Whether or not to apply separate blending operations for each attachment.
	 *
	 * If unset, only the first attachment blend states should be used.
	 */
	mslBool separateAttachmentBlendingEnable;

	/**
	 * @brief The blend states for each color attachment.
	 */
	mslBlendAttachmentState blendAttachments[MSL_MAX_ATTACHMENTS];

	/**
	 * @brief The constant blend factor.
	 */
	float blendConstants[4];
} mslBlendState;

/**
 * @brief Structure for holding the render states.
 */
typedef struct mslRenderState
{
	/**
	 * @brief The rasterization states.
	 */
	mslRasterizationState rasterizationState;

	/**
	 * @brief The multisample states.
	 */
	mslMultisampleState multisampleState;

	/**
	 * @brief The depth-stencil states.
	 */
	mslDepthStencilState depthStencilState;

	/**
	 * @brief The blending states.
	 */
	mslBlendState blendState;

	/**
	 * @brief The number of control points for each patch for tessellation.
	 */
	uint32_t patchControlPoints;
} mslRenderState;

/**
 * @brief Structure for holding the states used for a texture sampler.
 */
typedef struct mslSamplerState
{
	/**
	 * @brief The filter used for minification.
	 */
	mslFilter minFilter;

	/**
	 * @brief The filter used for magnification.
	 */
	mslFilter magFilter;

	/**
	 * @brief The filter used for mip-mapping.
	 */
	mslMipFilter mipFilter;

	/**
	 * @brief How to address the U (or S) texture coordinate.
	 */
	mslAddressMode addressModeU;

	/**
	 * @brief How to address the U (or T) texture coordinate.
	 */
	mslAddressMode addressModeV;

	/**
	 * @brief How to address the W (or R) texture coordinate.
	 */
	mslAddressMode addressModeW;

	/**
	 * @brief Bias to apply when calculating the mip-mapping LOD.
	 */
	float mipLodBias;

	/**
	 * @brief The maximum anisotropy factor to apply.
	 */
	float maxAnisotropy;

	/**
	 * @brief The minimum mip level to use.
	 */
	float minLod;

	/**
	 * @brief The maximum mip level to use.
	 */
	float maxLod;

	/**
	 * @brief The border color to apply.
	 */
	mslBorderColor borderColor;

	/**
	 * @brief The compare op for shadow samplers.
	 */
	mslCompareOp compareOp;
} mslSamplerState;

/**
 * @brief Structure holding info for an array within a uniform or attribute.
 */
typedef struct mslArrayInfo
{
	/**
	 * @brief The length of the array.
	 *
	 * This will be set to unknown if the size is unknown at compile time.
	 */
	uint32_t length;

	/**
	 * @brief The stride of the array.
	 */
	uint32_t stride;
} mslArrayInfo;

/**
 * @brief Structure describing a member of a user-defined struct
 */
typedef struct mslStructMember
{
	/**
	 * @brief The name of the element.
	 */
	const char* name;

	/**
	 * @brief The offset from the beginning of the structure.
	 */
	uint32_t offset;

	/**
	 * @brief The size of this member.
	 */
	uint32_t size;

	/**
	 * @brief The type of the element.
	 */
	mslType type;

	/**
	 * @brief If type is Struct, this is the index to the stucture the member refers to.
	 */
	uint32_t structIndex;

	/**
	 * @brief The number of array elements.
	 *
	 * The info for each array element can be queried from the API. This will be empty if this
	 * member isn't an array.
	 */
	uint32_t arrayElementCount;

	/**
	 * @brief True if type is a matrix type and is row major.
	 */
	bool rowMajor;
} mslStructMember;

/**
 * @brief Structure describing a user-defined struct.
 */
typedef struct mslStruct
{
	/**
	 * @brief The type name of the struct.
	 */
	const char* name;

	/**
	 * @brief The size of the structure.
	 *
	 * If an element contains an unknown array length, this will be the size of the known
	 * elements.
	 */
	uint32_t size;

	/**
	 * @brief The number of structure members.
	 *
	 * The info for each member can be queried from the API.
	 */
	uint32_t memberCount;
} mslStruct;

/**
 * @brief Structure defining a uniform.
 */
typedef struct mslUniform
{
	/**
	 * @brief The name of the uniform.
	 *
	 * If the uniform is a structure, it is the same as the structure name.
	 */
	const char* name;

	/**
	 * @brief The uniform type, describing how the uniform is bound.
	 */
	mslUniformType uniformType;

	/**
	 * @brief The type of the uniform value.
	 */
	mslType type;

	/**
	 * @brief If type is Struct, this is the index to the stucture the member refers to.
	 */
	uint32_t structIndex;

	/**
	 * @brief The number of array elements.
	 *
	 * The info for each array element can be queried from the API. This will be empty if this
	 * uniform isn't an array.
	 */
	uint32_t arrayElementCount;

	/**
	 * @brief The index of the descriptor set.
	 *
	 * This may be set to unknown if not set.
	 */
	uint32_t descriptorSet;

	/**
	 * @brief The binding index.
	 *
	 * This may be set to unknown if not set.
	 */
	uint32_t binding;

	/**
	 * @brief The input attachment index for subpass inputs.
	 *
	 * This may be set to unknown if not set.
	 */
	uint32_t inputAttachmentIndex;

	/**
	 * @brief The index of the sampler state to use for sampler states.
	 *
	 * This will be set to unknown for non-sampler types or when no sampler state is defined.
	 */
	uint32_t samplerIndex;
} mslUniform;

/**
 * @brief Structure describing a vertex input attribute.
 */
typedef struct mslAttribute
{
	/**
	 * @brief The name of the attribute.
	 */
	const char* name;

	/**
	 * @brief The type of the attribute.
	 */
	mslType type;

	/**
	 * @brief The number of array elements.
	 *
	 * The info for each array element can be queried from the API. This will be empty if this
	 * attribute isn't an array.
	 */
	uint32_t arrayElementCount;

	/**
	 * @brief The location of the attribute.
	 */
	uint32_t location;

	/**
	 * @brief The first component at location to use.
	 */
	uint32_t component;
} mslAttribute;

/**
 * @brief Structure describing a fragment color output.
 */
typedef struct mslFragmentOutput
{
	/**
	 * @brief The name of the output.
	 */
	const char* name;

	/**
	 * @brief The location of the output.
	 */
	uint32_t location;
} mslFragmentOutput;

/**
 * @brief Structure that holds the information about the a pipeline within the compiled result.
 */
typedef struct mslPipeline
{
	/**
	 * @brief The name of the pipeline.
	 */
	const char* name;

	/**
	 * @brief The number of structs used within the pipeline.
	 *
	 * The info for each struct can be queried from the API.
	 */
	uint32_t structCount;

	/**
	 * @brief The number of sampler states.
	 *
	 * The info for each sampler state can be queried from the API.
	 */
	uint32_t samplerStateCount;

	/**
	 * @brief The number of uniforms used within the pipeline.
	 *
	 * The info for each uniform can be queried from the API.
	 */
	uint32_t uniformCount;

	/**
	 * @brief The number of vertex attributes used within the pipeline.
	 *
	 * The info for each attribute can be queried from the API.
	 */
	uint32_t attributeCount;

	/**
	 * @brief The number of fragment outputs used within the pipeline.
	 *
	 * The info for each fragment output can be queried from the API.
	 */
	uint32_t fragmentOutputCount;

	/**
	 * @brief Index for the push constant structure.
	 *
	 * The push constant struct members will become individual uniforms for non-Vulkan targets.
	 * This is set to unknown if there are no push constants.
	 */
	uint32_t pushConstantStruct;

	/**
	 * @brief The shaders for the different pipeline stages.
	 *
	 * This can be indexed by the Stage enum. If the stage isn't used, the index will be set to
	 * unknown.
	 */
	uint32_t shaders[mslStage_Count];

	/**
	 * @brief The local size for the compute stage along the X, Y, and Z dimensions.
	 */
	uint32_t computeLocalSize[3];
} mslPipeline;

/**
 * @brief Typedef for a custom allocator function.
 *
 * If this function fails to allocate memory, it should set errno, either directly or indirectly.
 *
 * @param userData The user data for the allocator.
 * @param size The size to allocate.
 * @return The allocated memory, or NULL if the allocation failed.
 */
typedef void* (*mslAllocateFunction)(void* userData, size_t size);

/**
 * @brief Typedef for a custom free function.
 * @param userData The user data for the allocator.
 * @param ptr The pointer to free.
 */
typedef void (*mslFreeFunction)(void* userData, void* ptr);

/**
 * @brief Structure that holds the allocator info.
 */
typedef struct mslAllocator
{
	/**
	 * @brief The allocator function.
	 *
	 * This cannot be NULL.
	 */
	mslAllocateFunction allocateFunc;

	/**
	 * @brief The free function.
	 *
	 * If NULL, the memory won't be freed. This is useful for cases such memory arenas.
	 */
	mslFreeFunction freeFunc;

	/**
	 * @brief The user data to pass to the allocate and free functions.
	 */
	void* userData;
} mslAllocator;

/**
 * @brief Struct containing a size with a data pointer.
 */
typedef struct mslSizedData
{
	/**
	 * @brief size The size of the data.
	 */
	uint32_t size;

	/**
	 * @brief The data pointer.
	 */
	void* data;
} mslSizedData;

/**
 * @brief Typedef for a custom function for reading data from a stream.
 * @param userData The user data to read from.
 * @param buffer The buffer to read into.
 * @param size The number of bytes to read.
 * @return The number of read bytes, or 0 if an error occurred.
 */
typedef size_t (*mslReadFunction)(void* userData, void* buffer, size_t size);

/**
 * @brief Type for a shader module.
 *
 * This will contain all of the data for the shader module. It is designed so that only a single
 * allocation will be made for the data buffer and any metadata for accessing the data members.
 */
typedef struct mslModule mslModule;

#ifdef __cplusplus
}
#endif
