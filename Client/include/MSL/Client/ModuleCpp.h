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
#include <MSL/Client/TypesCpp.h>
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
	 * @brief Gets whether or not the bindings are adjustable in the shader module.
	 * @return True if bindings are adjustable.
	 */
	bool adjustableBindings() const;

	/**
	 * @brief Gets the number of pipelines within the shader module.
	 * @return The number of pipelines.
	 */
	uint32_t pipelineCount() const;

	/**
	 * @brief Gets the info for a pipeline within the shader module.
	 * @param[out] outPipeline The structure to hold the pipeline info.
	 * @param pipelineIndex The index of the pipeline.
	 * @return False if the parameters are incorrect.
	 */
	bool pipeline(Pipeline& outPipeline, uint32_t pipelineIndex) const;

	/**
	 * @brief Gets the info for a struct within a pipeline.
	 * @param[out] outStruct The structure to hold the struct info.
	 * @param pipelineIndex The index of the pipeline.
	 * @param structIndex The index of the struct within the pipeline.
	 * @return False if the parameters are incorrect.
	 */
	bool pipelineStruct(Struct& outStruct, uint32_t pipelineIndex, uint32_t structIndex) const;

	/**
	 * @brief Gets the info for a struct member within a pipeline.
	 * @param[out] outStructMember The structure to hold the struct member info.
	 * @param pipelineIndex The index of the pipeline.
	 * @param structIndex The index of the struct within the pipeline.
	 * @param structMemberIndex The index of the struct member within the struct.
	 * @return False if the parameters are incorrect.
	 */
	bool structMember(StructMember& outStructMember, uint32_t pipelineIndex, uint32_t structIndex,
		uint32_t structMemberIndex) const;

	/**
	 * @brief Gets the array info for a struct member within a pipeline.
	 * @param[out] outArrayInfo The structure to hold the struct member array info.
	 * @param pipelineIndex The index of the pipeline.
	 * @param structIndex The index of the struct within the pipeline.
	 * @param structMemberIndex The index of the struct member within the struct.
	 * @param arrayElement The array element to get the info for.
	 * @return False if the parameters are incorrect.
	 */
	bool structMemberArrayInfo(ArrayInfo& outArrayInfo, uint32_t pipelineIndex,
		uint32_t structIndex, uint32_t structMemberIndex, uint32_t arrayElement) const;

	/**
	 * @brief Gets the info for a sampler state within a pipeline.
	 * @param[out] outSamplerState The structure to hold the sampler state info.
	 * @param pipelineIndex The index of the pipeline.
	 * @param samplerStateIndex The index of the sampler state within the pipeline.
	 * @return False if the parameters are incorrect.
	 */
	bool samplerState(SamplerState& outSamplerState, uint32_t pipelineIndex,
		uint32_t samplerStateIndex) const;

	/**
	 * @brief Gets the info for a uniform within a pipeline.
	 * @param[out] outUniform The structure to hold the uniform info.
	 * @param pipelineIndex The index of the pipeline.
	 * @param uniformIndex The index of the uniform within the pipeline.
	 * @return False if the parameters are incorrect.
	 */
	bool uniform(Uniform& outUniform, uint32_t pipelineIndex, uint32_t uniformIndex) const;

	/**
	 * @brief Gets the array info for a uniform within a pipeline.
	 * @param[out] outArrayInfo The structure to hold the uniform array info.
	 * @param pipelineIndex The index of the pipeline.
	 * @param uniformIndex The index of the uniform within the pipeline.
	 * @param arrayElement The array element to get the info for.
	 * @return False if the parameters are incorrect.
	 */
	bool uniformArrayInfo(ArrayInfo& outArrayInfo, uint32_t pipelineIndex, uint32_t uniformIndex,
		uint32_t arrayElement) const;

	/**
	 * @brief Gets the info for a vertex attribute within a pipeline.
	 * @param[out] outAttribute The structure to hold the attribute info.
	 * @param pipelineIndex The index of the pipeline.
	 * @param attributeIndex The index of the attribute within the pipeline.
	 * @return False if the parameters are incorrect.
	 */
	bool attribute(Attribute& outAttribute, uint32_t pipelineIndex, uint32_t attributeIndex) const;

	/**
	 * @brief Gets the array length for a vertex attribute within a pipeline.
	 * @param pipelineIndex The index of the pipeline.
	 * @param attributeIndex The index of the attribute within the pipeline.
	 * @param arrayElement The array element to get the info for.
	 * @return The length of the array, or unkown if the parameters are incorrect.
	 */
	uint32_t attributeArrayLength(uint32_t pipelineIndex, uint32_t attributeIndex,
		uint32_t arrayElement) const;

	/**
	 * @brief Gets the render state for a pipeline within the module.
	 * @param[out] outRenderState The structure to hold the rneder state info.
	 * @param pipelineIndex The index of the pipeline.
	 * @return False if the parameters are incorrect.
	 */
	bool renderState(RenderState& outRenderState, uint32_t pipelineIndex) const;

	/**
	 * @brief Sets the descriptor set and binding for a uniform within a pipeline.
	 *
	 * This is only valid when the bindings are adjustable, which itself is only available for SPIR-V
	 * shaders. This will adjust the descriptor set and binding indices within the SPIR-V for each stage
	 * within the pipeline, as well as update the indices requested with mslModule_uniform().
	 *
	 * @param pipelineIndex The index of the pipeline.
	 * @param uniformIndex The index of the uniform within the pipeline.
	 * @param descriptorSet The new descriptor set to use.
	 * @param binding The new binding index set to use.
	 * @return False if the parameters are incorrect or the bindings aren't adjustable.
	 */
	bool setUniformBinding(uint32_t pipelineIndex, uint32_t uniformIndex, uint32_t descriptorSet,
		uint32_t binding);

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

static_assert(typeCount == mslType_Count, "Type enum mismatch.");
static_assert(stageCount == mslStage_Count, "Stage enum mismatch.");
static_assert(Bool::True == static_cast<Bool>(mslBool_True), "Bool enum mismatch.");
static_assert(PolygonMode::Point == static_cast<PolygonMode>(mslPolygonMode_Point),
	"PolygonMode enum mismatch.");
static_assert(CullMode::FrontAndBack == static_cast<CullMode>(mslCullMode_FrontAndBack),
	"CullMode enum mismatch.");
static_assert(FrontFace::Clockwise == static_cast<FrontFace>(mslFrontFace_Clockwise),
	"FrontFace enum mismatch.");
static_assert(StencilOp::DecrementAndWrap == static_cast<StencilOp>(mslStencilOp_DecrementAndWrap),
	"StencilOp enum mismatch.");
static_assert(CompareOp::Always == static_cast<CompareOp>(mslCompareOp_Always),
	"CompareOp enum mismatch.");
static_assert(BlendFactor::OneMinusSrc1Alpha ==
	static_cast<BlendFactor>(mslBlendFactor_OneMinusSrc1Alpha),
	"BlendFactor enum mismatch.");
static_assert(BlendOp::Max == static_cast<BlendOp>(mslBlendOp_Max), "BlendOp enum mismatch.");
static_assert(LogicOp::Set == static_cast<LogicOp>(mslLogicOp_Set), "LogicOp enum mismatch.");
static_assert(Filter::Linear == static_cast<Filter>(mslFilter_Linear), "Filter enum mismatch.");
static_assert(MipFilter::Anisotropic == static_cast<MipFilter>(mslMipFilter_Anisotropic),
	"MipFilter enum mismatch.");
static_assert(AddressMode::MirrorOnce == static_cast<AddressMode>(mslAddressMode_MirrorOnce),
	"AddressMode enum mismatch.");
static_assert(BorderColor::OpaqueIntOne == static_cast<BorderColor>(mslBorderColor_OpaqueIntOne),
	"BorderColor enum mismatch.");

static_assert(sizeof(RasterizationState) == sizeof(mslRasterizationState),
	"RasterizationState struct mismatch.");
static_assert(sizeof(MultisampleState) == sizeof(mslMultisampleState),
	"MultisampleState struct mismatch.");
static_assert(sizeof(StencilOpState) == sizeof(mslStencilOpState),
	"StencilOpState struct mismatch.");
static_assert(sizeof(DepthStencilState) == sizeof(mslDepthStencilState),
	"DepthStencilState struct mismatch.");
static_assert(sizeof(BlendAttachmentState) == sizeof(mslBlendAttachmentState),
	"BlendAttachmentState struct mismatch.");
static_assert(sizeof(BlendState) == sizeof(mslBlendState), "BlendState struct mismatch.");
static_assert(sizeof(RenderState) == sizeof(mslRenderState), "RenderState struct mismatch.");
static_assert(sizeof(SamplerState) == sizeof(mslSamplerState), "SamplerState struct mismatch.");
static_assert(sizeof(ArrayInfo) == sizeof(mslArrayInfo), "ArrayInfo struct mismatch.");
static_assert(sizeof(StructMember) == sizeof(mslStructMember), "StructMember struct mismatch.");
static_assert(sizeof(Struct) == sizeof(mslStruct), "Struct struct mismatch.");
static_assert(sizeof(Uniform) == sizeof(mslUniform), "Uniform struct mismatch.");
static_assert(sizeof(Attribute) == sizeof(mslAttribute), "Attribute struct mismatch.");
static_assert(sizeof(Pipeline) == sizeof(mslPipeline), "Pipeline struct mismatch.");

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
	m_size = mslModule_sizeof(size);
	m_module = mslModule_readStream(&readFunc, &stream, size, &alloc);
	return m_module != nullptr;
}

template <typename Allocator>
bool BasicModule<Allocator>::read(const void* data, size_t size)
{
	mslModule_destroy(m_module);
	m_module = nullptr;

	mslAllocator alloc = {&allocateFunc, &freeFunc, this};
	m_size = mslModule_sizeof(size);
	m_module = mslModule_readData(data, size, &alloc);
	return m_module != nullptr;
}

template <typename Allocator>
bool BasicModule<Allocator>::read(const char* fileName)
{
	mslModule_destroy(m_module);
	m_module = nullptr;

	std::ifstream stream(fileName, std::ios_base::binary);
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
bool BasicModule<Allocator>::adjustableBindings() const
{
	return mslModule_adjustableBindings(m_module);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::pipelineCount() const
{
	return mslModule_pipelineCount(m_module);
}

template <typename Allocator>
bool BasicModule<Allocator>::pipeline(Pipeline& outPipeline, uint32_t pipelineIndex) const
{
	return mslModule_pipeline(reinterpret_cast<mslPipeline*>(&outPipeline), m_module,
		pipelineIndex);
}

template <typename Allocator>
bool BasicModule<Allocator>::pipelineStruct(Struct& outStruct, uint32_t pipelineIndex,
	uint32_t structIndex) const
{
	return mslModule_struct(reinterpret_cast<mslStruct*>(&outStruct), m_module, pipelineIndex,
		structIndex);
}

template <typename Allocator>
bool BasicModule<Allocator>::structMember(StructMember& outStructMember, uint32_t pipelineIndex,
	uint32_t structIndex, uint32_t structMemberIndex) const
{
	return mslModule_structMember(reinterpret_cast<mslStructMember*>(&outStructMember), m_module,
		pipelineIndex, structIndex, structMemberIndex);
}

template <typename Allocator>
bool BasicModule<Allocator>::structMemberArrayInfo(ArrayInfo& outArrayInfo, uint32_t pipelineIndex,
	uint32_t structIndex, uint32_t structMemberIndex, uint32_t arrayElement) const
{
	return mslModule_structMemberArrayInfo(reinterpret_cast<mslArrayInfo*>(&outArrayInfo), m_module,
		pipelineIndex, structIndex, structMemberIndex, arrayElement);
}

template <typename Allocator>
bool BasicModule<Allocator>::samplerState(SamplerState& outSamplerState, uint32_t pipelineIndex,
	uint32_t samplerStateIndex) const
{
	return mslModule_samplerState(reinterpret_cast<mslSamplerState*>(&outSamplerState), m_module,
		pipelineIndex, samplerStateIndex);
}

template <typename Allocator>
bool BasicModule<Allocator>::uniform(Uniform& outUniform, uint32_t pipelineIndex,
	uint32_t uniformIndex) const
{
	return mslModule_uniform(reinterpret_cast<mslUniform*>(&outUniform), m_module, pipelineIndex,
		uniformIndex);
}

template <typename Allocator>
bool BasicModule<Allocator>::uniformArrayInfo(ArrayInfo& outArrayInfo, uint32_t pipelineIndex,
	uint32_t uniformIndex, uint32_t arrayElement) const
{
	return mslModule_uniformArrayInfo(reinterpret_cast<mslArrayInfo*>(&outArrayInfo), m_module,
		pipelineIndex, uniformIndex, arrayElement);
}

template <typename Allocator>
bool BasicModule<Allocator>::attribute(Attribute& outAttribute, uint32_t pipelineIndex,
	uint32_t attributeIndex) const
{
	return mslModule_attribute(reinterpret_cast<mslAttribute*>(&outAttribute), m_module,
		pipelineIndex, attributeIndex);
}

template <typename Allocator>
uint32_t BasicModule<Allocator>::attributeArrayLength(uint32_t pipelineIndex,
	uint32_t attributeIndex, uint32_t arrayElement) const
{
	return mslModule_attributeArrayLength(m_module, pipelineIndex, attributeIndex, arrayElement);
}

template <typename Allocator>
bool BasicModule<Allocator>::renderState(RenderState& outRenderState, uint32_t pipelineIndex) const
{
	return mslModule_renderState(reinterpret_cast<mslRenderState*>(&outRenderState), m_module,
		pipelineIndex);
}

template <typename Allocator>
bool BasicModule<Allocator>::setUniformBinding(uint32_t pipelineIndex, uint32_t uniformIndex,
	uint32_t descriptorSet, uint32_t binding)
{
	return mslModule_setUniformBinding(m_module, pipelineIndex, uniformIndex, descriptorSet,
		binding);
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
