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
#include <MSL/Client/Export.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @file
 * @brief Shader module loading implementation for C.
 *
 * Modules can be read by stream with mslModule_readStream(), data pointer with
 * mslModule_readData(), or file with mslModule_readFile(). When finished with a module, call
 * mslModule_destroy() to destroy it.
 *
 * The module will be created with a single allocation, the size of which can be queried with
 * mslModule_sizeof(). As a result of this implementation, most queries require computing an offset
 * within the buffer, including endian swaps on big-endian systems. This can be mitigated by making
 * sure that values are cached when using them. For example:
 *
 * @code
 * uint32_t pipelineCount = mslModule_pipelineCount(module);
 * for (uirnt32_t i = 0; i < pipelineCount; ++i)
 * {
 *     ...
 * }
 * @endcode
 *
 * When reading a module, errno will be set based on the error that occurred. Typical values are:
 * - EINVAL: invalid arguments.
 * - ENOMEM: failed to allocate memory.
 * - ENOENT: no file found.
 * - EACCESS: permission denied reading the file.
 * - EIO: IO error reading the file. This will also be set if the file isn't large enough to read
 *   the buffer size passed in.
 * - EILSEQ: invalid file format.
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
 * @brief Constant for no shader being set for a pipeline stage.
 */
#define MSL_NO_SHADER (uint32_t)-1

/**
 * @brief Constant for an unknown value.
 */
#define MSL_UNKNOWN (uint32_t)-1

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

/**
 * @brief Enum for the type of a uniform or attribute.
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
	mslType_Count                 ///< Number of types.
} mslType;

/**
 * @brief Enum for a stage within a shader pipeline.
 */
typedef enum mslStage
{
	mslStage_Vertex,                 ///< Vertex stage.
	mslStage_TessellationControl,    ///< Tessellation control stage.
	mslStage_TessellationEvaluation, ///< Tessellation evaluation stage.
	mslStage_Geometry,               ///< Geometry stage.
	mslStage_Fragment,               ///< Fragment stage.
	mslStage_Compute,                ///< Compute stage.
	mslStage_Count                   ///< Number of stages.
} mslStage;

/**
 * @brief Gets the size that will be allocated for a module.
 * @param dataSize THe size of the module data.
 * @return The allocated size for the module data and metadata.
 */
MSL_CLIENT_EXPORT size_t mslModule_sizeof(size_t dataSize);

/**
 * @brief Reads a shader module from a stream.
 *
 * This will read exactly size bytes from the stream. This is used in order to allocate the proper
 * amount of data up-front and guarantee only a single allocation is done.
 *
 * @param readFunc The function to read data from.
 * @param userData The user data to pass to the read function.
 * @param size The size of the data to read.
 * @param allocator The allocator. If NULL, malloc will be used instead.
 * @return The read shader module, or NULL if it couldn't be read. errno will be set on failure.
 */
MSL_CLIENT_EXPORT mslModule* mslModule_readStream(mslReadFunction readFunc, void* userData,
	size_t size, const mslAllocator* allocator);

/**
 * @brief Reads a shader module from a data buffer.
 *
 * This will copy the contents of the buffer into the created shader module.
 *
 * @param data The data buffer to read from.
 * @param size The size of the data to read.
 * @param allocator The allocator. If NULL, malloc will be used instead.
 * @return The read shader module, or NULL if it couldn't be read. errno will be set on failure.
 */
MSL_CLIENT_EXPORT mslModule* mslModule_readData(const void* data, size_t size,
	const mslAllocator* allocator);

/**
 * @brief Reads a shader module from a file.
 * @param fileName The name of the file to read from.
 * @param allocator The allocator. If NULL, malloc will be used instead.
 * @return The read shader module, or NULL if it couldn't be read. errno will be set on failure.
 */
MSL_CLIENT_EXPORT mslModule* mslModule_readFile(const char* fileName,
	const mslAllocator* allocator);

/**
 * @brief Gets the file version of the module.
 * @param module The shader module.
 * @return The file version.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_version(const mslModule* module);

/**
 * @brief Gets the target ID for a shader module.
 * @param module The shader module.
 * @return The target ID.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_targetId(const mslModule* module);

/**
 * @brief Gets the target version for a shader module.
 * @param module The shader module.
 * @return The target version.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_targetVersion(const mslModule* module);

/**
 * @brief Gets the number of pipelines within the shader module.
 * @param module The shader module.
 * @return The number of pipelines.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_pipelineCount(const mslModule* module);

/**
 * @brief Gets the name of a pipeline within the shader module.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @return The name of the pipeline.
 */
MSL_CLIENT_EXPORT const char* mslModule_pipelineName(const mslModule* module, uint32_t pipeline);

/**
 * @brief Gets the shader index of a pipeline stage.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param stage The stage to get the shader from.
 * @return The index of the shader for the stage.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_pipelineShader(const mslModule* module, uint32_t pipeline,
	mslStage stage);

/**
 * @brief Gets the number of uniforms within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @return The number of uniforms for the pipeline.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_uniformCount(const mslModule* module, uint32_t pipeline);

/**
 * @brief Gets the name of a uniform within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param uniform The index of the uniform.
 * @return The name of the uniform.
 */
MSL_CLIENT_EXPORT const char* mslModule_uniformName(const mslModule* module, uint32_t pipeline,
	uint32_t uniform);

/**
 * @brief Gets the type of a uniform within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param uniform The index of the uniform.
 * @return The type of the uniform.
 */
MSL_CLIENT_EXPORT mslType mslModule_uniformType(const mslModule* module, uint32_t pipeline,
	uint32_t uniform);

/**
 * @brief Gets the block index of a uniform within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param uniform The index of the uniform.
 * @return The block index of the uniform, or MSL_UNKNOWN if not part of a block.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_uniformBlockIndex(const mslModule* module, uint32_t pipeline,
	uint32_t uniform);

/**
 * @brief Gets the buffer offset of a uniform within a pipeline.
 * @remark This is based on the original GLSL code, and may not be the same when cross-compiled
 * to other targets that use different alignment. The offsets should be queried at runtime for
 * targets that have different alignment rules.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param uniform The index of the uniform.
 * @return The byte offset within the buffer, or MSL_UNKNOWN if unknown.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_uniformBufferOffset(const mslModule* module, uint32_t pipeline,
	uint32_t uniform);

/**
 * @brief Gets the number of array elements for a uniform within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param uniform The index of the uniform.
 * @return The number of array elements. This will be a minimum of 1 if the uniform is valid.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_uniformElements(const mslModule* module, uint32_t pipeline,
	uint32_t uniform);

/**
 * @brief Gets the number of uniform blocks within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @return The number of uniform blocks for the pipeline.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_uniformBlockCount(const mslModule* module, uint32_t pipeline);

/**
 * @brief Gets the name of a uniform block within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param block The index of the uniform block.
 * @return The name of the uniform block.
 */
MSL_CLIENT_EXPORT const char* mslModule_uniformBlockName(const mslModule* module, uint32_t pipeline,
	uint32_t block);

/**
 * @brief Gets the size of a uniform block within a pipeline.
 * @remark This is based on the original GLSL code, and may not be the same when cross-compiled
 * to other targets that use different alignment. The size should be queried at runtime for
 * targets that have different alignment rules.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param block The index of the uniform block.
 * @return The size in bytes of the uniform block, or MSL_UNKNOWN if unknown.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_uniformBlockSize(const mslModule* module, uint32_t pipeline,
	uint32_t block);

/**
 * @brief Gets the number of vertex attributes within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @return The number of vertex attributes for the pipeline.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_attributeCount(const mslModule* module, uint32_t pipeline);

/**
 * @brief Gets the name of a vertex attribute within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param attribute The index of the vertex attribute.
 * @return The name of the vertex attribute.
 */
MSL_CLIENT_EXPORT const char* mslModule_attributeName(const mslModule* module, uint32_t pipeline,
	uint32_t attribute);

/**
 * @brief Gets the type of a vertex attribute within a pipeline.
 * @param module The shader module.
 * @param pipeline The index of the pipeline.
 * @param attribute The index of the vertex attribute.
 * @return The type of the vertex attribute.
 */
MSL_CLIENT_EXPORT mslType mslModule_attributeType(const mslModule* module, uint32_t pipeline,
	uint32_t attribute);

/**
 * @brief Gets number of shaders within the module.
 * @param module The shader module.
 * @return The number of shaders.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_shaderCount(const mslModule* module);

/**
 * @brief Gets the size of a shader within the module.
 * @param module The shader module.
 * @param shader The index of the shader.
 * @return The size of the shader in bytes.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_shaderSize(const mslModule* module, uint32_t shader);

/**
 * @brief Gets the data of a shader within the module.
 * @param module The shader module.
 * @param shader The index of the shader.
 * @return The data for the shader.
 */
MSL_CLIENT_EXPORT const void* mslModule_shaderData(const mslModule* module, uint32_t shader);

/**
 * @brief Gets the size of the shared data within the module.
 * @param module The shader module.
 * @return The size of the shared data in bytes.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_sharedDataSize(const mslModule* module);

/**
 * @brief Gets the shared data within the module.
 * @param module The shader module.
 * @return The shared data.
 */
MSL_CLIENT_EXPORT const void* mslModule_sharedData(const mslModule* module);

/**
 * @brief Destroys a shader module.
 * @param module The module to destroy.
 */
MSL_CLIENT_EXPORT void mslModule_destroy(mslModule* module);

#ifdef __cplusplus
}
#endif
