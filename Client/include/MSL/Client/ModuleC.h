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
#include <MSL/Client/TypesC.h>

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
 * @brief Gets whether or not the bindings are adjustable in the shader module.
 * @param module The shader module.
 * @return True if bindings are adjustable.
 */
MSL_CLIENT_EXPORT bool mslModule_adjustableBindings(const mslModule* module);

/**
 * @brief Gets the number of pipelines within the shader module.
 * @param module The shader module.
 * @return The number of pipelines.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_pipelineCount(const mslModule* module);

/**
 * @brief Gets the info for a pipeline within the shader module.
 * @param[out] outPipeline The structure to hold the pipeline info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_pipeline(mslPipeline* outPipeline, const mslModule* module,
	uint32_t pipelineIndex);

/**
 * @brief Gets the info for a struct within a pipeline.
 * @param[out] outStruct The structure to hold the struct info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param structIndex The index of the struct within the pipeline.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_struct(mslStruct* outStruct, const mslModule* module,
	uint32_t pipelineIndex, uint32_t structIndex);

/**
 * @brief Gets the info for a struct member within a pipeline.
 * @param[out] outStructMember The structure to hold the struct member info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param structIndex The index of the struct within the pipeline.
 * @param structMemberIndex The index of the struct member within the struct.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_structMember(mslStructMember* outStructMember,
	const mslModule* module, uint32_t pipelineIndex, uint32_t structIndex,
	uint32_t structMemberIndex);

/**
 * @brief Gets the array info for a struct member within a pipeline.
 * @param[out] outArrayInfo The structure to hold the struct member array info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param structIndex The index of the struct within the pipeline.
 * @param structMemberIndex The index of the struct member within the struct.
 * @param arrayElement The array element to get the info for.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_structMemberArrayInfo(mslArrayInfo* outArrayInfo,
	const mslModule* module, uint32_t pipelineIndex, uint32_t structIndex,
	uint32_t structMemberIndex, uint32_t arrayElement);

/**
 * @brief Gets the info for a sampler state within a pipeline.
 * @param[out] outSamplerState The structure to hold the sampler state info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param samplerStateIndex The index of the sampler state within the pipeline.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_samplerState(mslSamplerState* outSamplerState,
	const mslModule* module, uint32_t pipelineIndex, uint32_t samplerStateIndex);

/**
 * @brief Gets the info for a uniform within a pipeline.
 * @param[out] outUniform The structure to hold the uniform info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param uniformIndex The index of the uniform within the pipeline.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_uniform(mslUniform* outUniform,
	const mslModule* module, uint32_t pipelineIndex, uint32_t uniformIndex);

/**
 * @brief Gets the array info for a uniform within a pipeline.
 * @param[out] outArrayInfo The structure to hold the uniform array info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param uniformIndex The index of the uniform within the pipeline.
 * @param arrayElement The array element to get the info for.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_uniformArrayInfo(mslArrayInfo* outArrayInfo,
	const mslModule* module, uint32_t pipelineIndex, uint32_t uniformIndex, uint32_t arrayElement);

/**
 * @brief Gets the info for a vertex attribute within a pipeline.
 * @param[out] outAttribute The structure to hold the attribute info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param attributeIndex The index of the attribute within the pipeline.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_attribute(mslAttribute* outAttribute,
	const mslModule* module, uint32_t pipelineIndex, uint32_t attributeIndex);

/**
 * @brief Gets the array info for a vertex attribute within a pipeline.
 * @param[out] outArrayInfo The structure to hold the uniform array info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param attributeIndex The index of the attribute within the pipeline.
 * @param arrayElement The array element to get the info for.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_attributeArrayInfo(mslArrayInfo* outArrayInfo,
	const mslModule* module, uint32_t pipelineIndex, uint32_t attributeIndex,
	uint32_t arrayElement);

/**
 * @brief Gets the render state for a pipeline within the module.
 * @param[out] outRenderState The structure to hold the rneder state info.
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @return False if the parameters are incorrect.
 */
MSL_CLIENT_EXPORT bool mslModule_renderState(mslRenderState* outRenderState,
	const mslModule* module, uint32_t pipelineIndex);

/**
 * @brief Sets the descriptor set and binding for a uniform within a pipeline.
 *
 * This is only valid when the bindings are adjustable, which itself is only available for SPIR-V
 * shaders. This will adjust the descriptor set and binding indices within the SPIR-V for each stage
 * within the pipeline, as well as update the indices requested with mslModule_uniform().
 *
 * @param module The shader module.
 * @param pipelineIndex The index of the pipeline.
 * @param uniformIndex The index of the uniform within the pipeline.
 * @param descriptorSet The new descriptor set to use.
 * @param binding The new binding index set to use.
 * @return False if the parameters are incorrect or the bindings aren't adjustable.
 */
MSL_CLIENT_EXPORT bool mslModule_setUniformBinding(mslModule* module, uint32_t pipelineIndex,
	uint32_t uniformIndex, uint32_t descriptorSet, uint32_t binding);

/**
 * @brief Gets number of shaders within the module.
 * @param module The shader module.
 * @return The number of shaders.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_shaderCount(const mslModule* module);

/**
 * @brief Gets the size of a shader within the module.
 * @param module The shader module.
 * @param shaderIndex The index of the shader.
 * @return The size of the shader in bytes.
 */
MSL_CLIENT_EXPORT uint32_t mslModule_shaderSize(const mslModule* module, uint32_t shaderIndex);

/**
 * @brief Gets the data of a shader within the module.
 * @param module The shader module.
 * @param shaderIndex The index of the shader.
 * @return The data for the shader.
 */
MSL_CLIENT_EXPORT const void* mslModule_shaderData(const mslModule* module, uint32_t shaderIndex);

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
