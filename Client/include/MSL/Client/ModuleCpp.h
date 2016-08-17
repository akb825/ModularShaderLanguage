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
#include <MSL/Client/ModuleC.h>
#include <fstream>
#include <memory>

/**
 * @file
 * @brief Shader module loading implementation for C++.
 */

namespace msl
{

/**
 * @brief Class for a shader module.
 *
 * Modules can be read with read(), reading from a stream, data pointer, or file.
 *
 * The module will be created with a single allocation, the size of which can be queried with
 * mslModule_sizeof(). As a result of this implementation, most queries require computing an offset
 * within the buffer, including endian swaps on big-endian systems. This can be mitigated by making
 * sure that values are cached when using them. For example:
 *
 * @code
 * uint32_t pipelineCount = module.pipelineCount();
 * for (uirnt32_t i = 0; i < pipelineCount; ++i)
 * {
 *     ...
 * }
 * @endcode
 *
 * @tparam allocator The allocator
 */
template <typename Allocator>
class BasicModule
{
public:
	/**
	 * @brief The type of the allocator.
	 */
	using AllocatorType = Allocator;

	/**
	* @brief Enum for the type of a uniform or attribute.
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
	};

	/**
	 * @brief Constant for the number of types.
	 */
	static const unsigned int typeCount = static_cast<unsigned int>(Type::USampler2DRect) + 1;

	/**
	* @brief Enum for a stage within a shader pipeline.
	*/
	enum class Stage
	{
		Vertex,                 ///< Vertex stage.
		TessellationControl,    ///< Tessellation control stage.
		TessellationEvaluation, ///< Tessellation evaluation stage.
		Geometry,               ///< Geometry stage.
		Fragment,               ///< Fragment stage.
		Compute                 ///< Compute stage.
	};

	/**
	 * @brief Constant for the number of stages.
	 */
	static const unsigned int stageCount = static_cast<unsigned int>(Stage::Compute) + 1;

	/**
	 * @brief Constant for the file version.
	 */
	static const uint32_t currentVersion = MSL_MODULE_VERSION;

	/**
	 * @brief Constant for no shader being set.
	 */
	static const uint32_t noShader = MSL_NO_SHADER;

	/**
	 * @brief Constant for no known value.
	 */
	static const uint32_t unknown = MSL_UNKNOWN;

	/**
	 * @brief Constructs this with the allocator.
	 * @param alloc The allocator.
	 */
	explicit BasicModule(AllocatorType alloc = AllocatorType());
	~BasicModule();

	BasicModule(const BasicModule&) = delete;
	BasicModule& operator=(const BasicModule&) = delete;

	/**
	 * @brief Gets the allocator.
	 * @return The allocator.
	 */
	AllocatorType& allocator();

	/** @copydoc allocator() */
	const AllocatorType& allocator() const;

	/**
	 * @brief Reads the module from a stream.
	 *
	 * This will seek to find the size before reading the data. The previous contents of the module
	 * will be destroyed.
	 *
	 * @param stream The stream to read from.
	 * @return False if the module couldn't be read.
	 */
	bool read(std::istream& stream);

	/**
	 * @brief Reads the module from a stream.
	 *
	 * This will use the size provided as a parameter. The previous contents of the module will be
	 * destroyed.
	 *
	 * @param stream The stream to read from.
	 * @param size The size of the data to read.
	 * @return False if the module couldn't be read.
	 */
	bool read(std::istream& stream, size_t size);

	/**
	 * @brief Reads the module from a data buffer.
	 *
	 * The previous contents of the module will be destroyed.
	 *
	 * @param data The data to read from.
	 * @param size The size of the data.
	 * @return False if the module couldn't be read.
	 */
	bool read(const void* data, size_t size);

	/**
	 * @brief Reads the module from a file.
	 *
	 * The previous contents of the module will be destroyed.
	 *
	 * @param fileName The name of the file to read from.
	 * @return False if the module couldn't be read.
	 */
	bool read(const char* fileName);

	/**
	 * @brief Reads the module from a file.
	 *
	 * The previous contents of the module will be destroyed.
	 *
	 * @param fileName The name of the file to read from.
	 * @return False if the module couldn't be read.
	 */
	bool read(const std::string& fileName);

	/**
	 * @brief Gets the file version of the module.
	 * @return The file version.
	 */
	uint32_t version() const;

	/**
	 * @brief Gets the target ID for a shader module.
	 * @return The target ID.
	 */
	uint32_t targetId() const;

	/**
	 * @brief Gets the target version for a shader module.
	 * @return The target version.
	 */
	uint32_t targetVersion() const;

	/**
	 * @brief Gets the number of pipelines within the shader module.
	 * @return The number of pipelines.
	 */
	uint32_t pipelineCount() const;

	/**
	 * @brief Gets the name of a pipeline within the shader module.
	 * @param pipeline The index of the pipeline.
	 * @return The name of the pipeline.
	 */
	const char* pipelineName(uint32_t pipeline) const;

	/**
	 * @brief Gets the shader index of a pipeline stage.
	 * @param pipeline The index of the pipeline.
	 * @param stage The stage to get the shader from.
	 * @return The index of the shader for the stage.
	 */
	uint32_t pipelineShader(uint32_t pipeline, Stage stage) const;

	/**
	 * @brief Gets the number of uniforms within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @return The number of uniforms for the pipeline.
	 */
	uint32_t uniformCount(uint32_t pipeline) const;

	/**
	 * @brief Gets the name of a uniform within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @param uniform The index of the uniform.
	 * @return The name of the uniform.
	 */
	const char* uniformName(uint32_t pipeline, uint32_t uniform) const;

	/**
	 * @brief Gets the type of a uniform within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @param uniform The index of the uniform.
	 * @return The type of the uniform.
	 */
	Type uniformType(uint32_t pipeline, uint32_t uniform) const;

	/**
	 * @brief Gets the block index of a uniform within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @param uniform The index of the uniform.
	 * @return The block index of the uniform, or MSL_UNKNOWN if not part of a block.
	 */
	uint32_t uniformBlockIndex(uint32_t pipeline, uint32_t uniform) const;

	/**
	 * @brief Gets the buffer offset of a uniform within a pipeline.
	 * @remark This is based on the original GLSL code, and may not be the same when cross-compiled
	 * to other targets that use different alignment. The offsets should be queried at runtime for
	 * targets that have different alignment rules.
	 * @param pipeline The index of the pipeline.
	 * @param uniform The index of the uniform.
	 * @return The byte offset within the buffer, or MSL_UNKNOWN if unknown.
	 */
	uint32_t uniformBufferOffset(uint32_t pipeline, uint32_t uniform) const;

	/**
	 * @brief Gets the number of array elements for a uniform within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @param uniform The index of the uniform.
	 * @return The number of array elements. This will be a minimum of 1 if the uniform is valid.
	 */
	uint32_t uniformElements(uint32_t pipeline, uint32_t uniform) const;

	/**
	 * @brief Gets the number of uniform blocks within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @return The number of uniform blocks for the pipeline.
	 */
	uint32_t uniformBlockCount(uint32_t pipeline) const;

	/**
	 * @brief Gets the name of a uniform block within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @param block The index of the uniform block.
	 * @return The name of the uniform block.
	 */
	const char* uniformBlockName(uint32_t pipeline, uint32_t block) const;

	/**
	 * @brief Gets the size of a uniform block within a pipeline.
	 * @remark This is based on the original GLSL code, and may not be the same when cross-compiled
	 * to other targets that use different alignment. The size should be queried at runtime for
	 * targets that have different alignment rules.
	 * @param pipeline The index of the pipeline.
	 * @param block The index of the uniform block.
	 * @return The size in bytes of the uniform block, or MSL_UNKNOWN if unknown.
	 */
	uint32_t uniformBlockSize(uint32_t pipeline, uint32_t block) const;

	/**
	 * @brief Gets the number of vertex attributes within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @return The number of vertex attributes for the pipeline.
	 */
	uint32_t attributeCount(uint32_t pipeline) const;

	/**
	 * @brief Gets the name of a vertex attribute within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @param attribute The index of the vertex attribute.
	 * @return The name of the vertex attribute.
	 */
	const char* attributeName(uint32_t pipeline, uint32_t attribute) const;

	/**
	 * @brief Gets the type of a vertex attribute within a pipeline.
	 * @param pipeline The index of the pipeline.
	 * @param attribute The index of the vertex attribute.
	 * @return The type of the vertex attribute.
	 */
	Type attributeType(uint32_t pipeline, uint32_t attribute) const;

	/**
	 * @brief Gets number of shaders within the module.
	 * @return The number of shaders.
	 */
	uint32_t shaderCount() const;

	/**
	 * @brief Gets the size of a shader within the module.
	 * @param shader The index of the shader.
	 * @return The size of the shader in bytes.
	 */
	uint32_t shaderSize(uint32_t shader) const;

	/**
	 * @brief Gets the data of a shader within the module.
	 * @param shader The index of the shader.
	 * @return The data for the shader.
	 */
	const void* shaderData(uint32_t shader) const;

	/**
	 * @brief Gets the size of the shared data within the module.
	 * @return The size of the shared data in bytes.
	 */
	uint32_t sharedDataSize() const;

	/**
	 * @brief Gets the shared data within the module.
	 * @return The shared data.
	 */
	const void* sharedData() const;

private:
	static void* allocateFunc(void* userData, size_t size);
	static void freeFunc(void* userData, void* ptr);
	static size_t readFunc(void* userData, void* buffer, size_t size);

	mslModule* m_module;
	size_t m_size;
	AllocatorType m_allocator;
};

/**
 * @brief Typedef for a module using the default allocator.
 */
using Module = BasicModule<std::allocator<uint8_t>>;

static_assert(Module::typeCount == mslType_Count, "Type enum mismatch.");
static_assert(Module::stageCount == mslStage_Count, "Stage enum mismatch.");

template <typename Allocator>
BasicModule<Allocator>::BasicModule(AllocatorType alloc)
	: m_module(nullptr)
	, m_allocator(std::move(alloc))
{
}

template <typename Allocator>
BasicModule<Allocator>::~BasicModule()
{
	mslModule_destroy(m_module);
}

template <typename Allocator>
Allocator& BasicModule<Allocator>::allocator()
{
	return m_allocator;
}

template <typename Allocator>
const Allocator& BasicModule<Allocator>::allocator() const
{
	return m_allocator;
}

template <typename Allocator>
bool BasicModule<Allocator>::read(std::istream& stream)
{
	mslModule_destroy(m_module);
	m_module = nullptr;

	if (!stream.seekg(0, std::istream::end))
		return false;
	size_t size = static_cast<size_t>(stream.tellg());
	if (!stream.seekg(0))
		return false;

	return read(stream, size);
}

template <typename Allocator>
bool BasicModule<Allocator>::read(std::istream& stream, size_t size)
{
	mslModule_destroy(m_module);
	m_module = nullptr;

	mslAllocator alloc = {&allocateFunc, &freeFunc, this};
	m_module = mslModule_readStream(&readFunc, &stream, size, &alloc);
	m_size = mslModule_sizeof(size);
	return m_module != nullptr;
}

template <typename Allocator>
bool BasicModule<Allocator>::read(const void* data, size_t size)
{
	mslModule_destroy(m_module);
	m_module = nullptr;

	mslAllocator alloc = {&allocateFunc, &freeFunc, this};
	m_module = mslModule_readData(data, size, &alloc);
	m_size = mslModule_sizeof(size);
	return m_module != nullptr;
}

template <typename Allocator>
bool BasicModule<Allocator>::read(const char* fileName)
{
	mslModule_destroy(m_module);
	m_module = nullptr;

	std::ifstream stream(fileName);
	if (!stream.is_open())
		return false;

	return read(stream);
}

template <typename Allocator>
bool BasicModule<Allocator>::read(const std::string& fileName)
{
	return read(fileName.c_str());
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::version() const
{
	return mslModule_version(m_module);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::targetId() const
{
	return mslModule_targetId(m_module);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::targetVersion() const
{
	return mslModule_targetVersion(m_module);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::pipelineCount() const
{
	return mslModule_pipelineCount(m_module);
}

template <typename Allocator>
const char* BasicModule<Allocator>::pipelineName(uint32_t pipeline) const
{
	return mslModule_pipelineName(m_module, pipeline);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::pipelineShader(uint32_t pipeline, Stage stage) const
{
	return mslModule_pipelineShader(m_module, pipeline, static_cast<mslStage>(stage));
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::uniformCount(uint32_t pipeline) const
{
	return mslModule_uniformCount(m_module, pipeline);
}

template <typename Allocator>
const char* BasicModule<Allocator>::uniformName(uint32_t pipeline, uint32_t uniform) const
{
	return mslModule_uniformName(m_module, pipeline, uniform);
}

template <typename Allocator>
auto BasicModule<Allocator>::uniformType(uint32_t pipeline, uint32_t uniform) const -> Type
{
	return static_cast<Type>(mslModule_uniformType(m_module, pipeline, uniform));
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::uniformBlockIndex(uint32_t pipeline, uint32_t uniform) const
{
	return mslModule_uniformBlockIndex(m_module, pipeline, uniform);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::uniformBufferOffset(uint32_t pipeline, uint32_t uniform) const
{
	return mslModule_uniformBufferOffset(m_module, pipeline, uniform);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::uniformElements(uint32_t pipeline, uint32_t uniform) const
{
	return mslModule_uniformElements(m_module, pipeline, uniform);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::uniformBlockCount(uint32_t pipeline) const
{
	return mslModule_uniformBlockCount(m_module, pipeline);
}

template <typename Allocator>
const char* BasicModule<Allocator>::uniformBlockName(uint32_t pipeline, uint32_t block) const
{
	return mslModule_uniformBlockName(m_module, pipeline, block);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::uniformBlockSize(uint32_t pipeline, uint32_t block) const
{
	return mslModule_uniformBlockSize(m_module, pipeline, block);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::attributeCount(uint32_t pipeline) const
{
	return mslModule_attributeCount(m_module, pipeline);
}

template <typename Allocator>
const char* BasicModule<Allocator>::attributeName(uint32_t pipeline, uint32_t attribute) const
{
	return mslModule_attributeName(m_module, pipeline, attribute);
}

template <typename Allocator>
auto BasicModule<Allocator>::attributeType(uint32_t pipeline, uint32_t attribute) const -> Type
{
	return static_cast<Type>(mslModule_attributeType(m_module, pipeline, attribute));
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::shaderCount() const
{
	return mslModule_shaderCount(m_module);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::shaderSize(uint32_t shader) const
{
	return mslModule_shaderSize(m_module, shader);
}

template <typename Allocator>
const void* BasicModule<Allocator>::shaderData(uint32_t shader) const
{
	return mslModule_shaderData(m_module, shader);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::sharedDataSize() const
{
	return mslModule_sharedDataSize(m_module);
}

template <typename Allocator>
const void* BasicModule<Allocator>::sharedData() const
{
	return mslModule_sharedData(m_module);
}

template <typename Allocator>
void* BasicModule<Allocator>::allocateFunc(void* userData, size_t size)
{
	return reinterpret_cast<AllocatorType*>(userData)->allocate(size);
}

template <typename Allocator>
void BasicModule<Allocator>::freeFunc(void* userData, void* ptr)
{
	BasicModule* thisPtr = reinterpret_cast<BasicModule*>(userData);
	thisPtr->m_allocator.deallocate(reinterpret_cast<uint8_t*>(ptr), thisPtr->m_size);
}

template <typename Allocator>
size_t BasicModule<Allocator>::readFunc(void* userData, void* buffer, size_t size)
{
	std::istream& stream = *reinterpret_cast<std::istream*>(userData);
	stream.read(reinterpret_cast<char*>(buffer), size);
	return stream.gcount();
}

} // namespace msl
