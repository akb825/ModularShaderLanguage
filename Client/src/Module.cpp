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

#include <MSL/Client/ModuleC.h>
#include "mslb_generated.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static_assert(static_cast<unsigned int>(mslb::Type::MAX) == mslType_Count - 1,
	"Type enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::UniformType::MAX) ==
	static_cast<int>(mslUniformType_SubpassInput),
	"UniformType enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::Bool::MAX) == static_cast<int>(mslBool_True),
	"Bool enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::PolygonMode::MAX) == static_cast<int>(mslPolygonMode_Point),
	"PolygonMode enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::CullMode::MAX) == static_cast<int>(mslCullMode_FrontAndBack),
	"CullMode enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::FrontFace::MAX) == static_cast<int>(mslFrontFace_Clockwise),
	"FrontFace enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::StencilOp::MAX) ==
	static_cast<int>(mslStencilOp_DecrementAndWrap),
	"StencilOp enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::CompareOp::MAX) == static_cast<int>(mslCompareOp_Always),
	"CompareOp enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::BlendFactor::MAX) ==
	static_cast<int>(mslBlendFactor_OneMinusSrc1Alpha),
	"BlendFactor enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::BlendOp::MAX) == static_cast<int>(mslBlendOp_Max),
	"BlendOp enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::LogicOp::MAX) == static_cast<int>(mslLogicOp_Set),
	"LogicOp enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::Filter::MAX) == static_cast<int>(mslFilter_Linear),
	"Filter enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::MipFilter::MAX) == static_cast<int>(mslMipFilter_Anisotropic),
	"MipFilter enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::AddressMode::MAX) == static_cast<int>(mslAddressMode_MirrorOnce),
	"AddressMode enum mismatch between flatbuffer and C.");
static_assert(static_cast<int>(mslb::BorderColor::MAX) ==
	static_cast<int>(mslBorderColor_OpaqueIntOne),
	"BorderColor enum mismatch between flatbuffer and C.");

static int invalidFormatErrno = EILSEQ;

static bool enumInRange(mslb::Type value)
{
	return value >= mslb::Type::MIN && value <= mslb::Type::MAX;
}

static bool enumInRange(mslb::UniformType value)
{
	return value >= mslb::UniformType::MIN && value <= mslb::UniformType::MAX;
}

static bool enumInRange(mslb::Bool value)
{
	return value >= mslb::Bool::MIN && value <= mslb::Bool::MAX;
}

static bool enumInRange(mslb::PolygonMode value)
{
	return value >= mslb::PolygonMode::MIN && value <= mslb::PolygonMode::MAX;
}

static bool enumInRange(mslb::CullMode value)
{
	return value >= mslb::CullMode::MIN && value <= mslb::CullMode::MAX;
}

static bool enumInRange(mslb::FrontFace value)
{
	return value >= mslb::FrontFace::MIN && value <= mslb::FrontFace::MAX;
}

static bool enumInRange(mslb::StencilOp value)
{
	return value >= mslb::StencilOp::MIN && value <= mslb::StencilOp::MAX;
}

static bool enumInRange(mslb::CompareOp value)
{
	return value >= mslb::CompareOp::MIN && value <= mslb::CompareOp::MAX;
}

static bool enumInRange(mslb::BlendFactor value)
{
	return value >= mslb::BlendFactor::MIN && value <= mslb::BlendFactor::MAX;
}

static bool enumInRange(mslb::BlendOp value)
{
	return value >= mslb::BlendOp::MIN && value <= mslb::BlendOp::MAX;
}

static bool enumInRange(mslb::LogicOp value)
{
	return value >= mslb::LogicOp::MIN && value <= mslb::LogicOp::MAX;
}

static bool enumInRange(mslb::Filter value)
{
	return value >= mslb::Filter::MIN && value <= mslb::Filter::MAX;
}

static bool enumInRange(mslb::MipFilter value)
{
	return value >= mslb::MipFilter::MIN && value <= mslb::MipFilter::MAX;
}

static bool enumInRange(mslb::AddressMode value)
{
	return value >= mslb::AddressMode::MIN && value <= mslb::AddressMode::MAX;
}

static bool enumInRange(mslb::BorderColor value)
{
	return value >= mslb::BorderColor::MIN && value <= mslb::BorderColor::MAX;
}

struct mslModule
{
	mslAllocator allocator;
	mslb::Module* module;
	uint8_t data[];
};

static bool canUseAllocator(const mslAllocator* allocator)
{
	return !allocator || allocator->allocateFunc;
}

static mslModule* createModule(size_t size, const mslAllocator* allocator)
{
	size_t totalSize = sizeof(mslModule) + size;
	mslModule* module;
	if (allocator)
	{
		module = reinterpret_cast<mslModule*>(
			allocator->allocateFunc(allocator->userData, totalSize));
		if (!module)
			return nullptr;

		module->allocator = *allocator;
	}
	else
	{
		module = reinterpret_cast<mslModule*>(malloc(totalSize));
		if (!module)
			return nullptr;

		memset(&module->allocator, 0, sizeof(module->allocator));
	}

	return module;
}

static void fixupModule(mslModule* module)
{
	// Swap if big endian and SPIR-V since it uses 32-bit values.
	bool swap = !FLATBUFFERS_LITTLEENDIAN &&
		module->module->targetId() == MSL_CREATE_ID('S', 'P', 'R', 'V');
	if (!swap)
		return;

	for (uint32_t i = 0; i < module->module->shaders()->size(); ++i)
	{
		auto& shaderData = *(*module->module->shaders())[i]->data();
		uint32_t* shaderData32 = const_cast<uint32_t*>(
			reinterpret_cast<const uint32_t*>(shaderData.data()));
		size_t shaderData32Size = shaderData.size()/sizeof(uint32_t);
		for (uint32_t j = 0; j < shaderData32Size; ++j)
			shaderData32[j] = flatbuffers::EndianScalar(shaderData32[j]);
	}
}

static size_t readFile(void* userData, void* buffer, size_t size)
{
	FILE* file = reinterpret_cast<FILE*>(userData);
	return fread(buffer, sizeof(uint8_t), size, file);
}

static bool isStencilOpStateValid(const mslb::StencilOpState& state)
{
	if (!enumInRange(state.failOp()))
		return false;
	if (!enumInRange(state.passOp()))
		return false;
	if (!enumInRange(state.depthFailOp()))
		return false;
	if (!enumInRange(state.compareOp()))
		return false;
	return true;
}

static bool isValid(const void* data, size_t size)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!mslb::VerifyModuleBuffer(verifier))
		return false;

	const mslb::Module* module = mslb::GetModule(data);
	if (module->version() > MSL_MODULE_VERSION)
		return false;

	bool isSpirV = module->targetId() != MSL_CREATE_ID('S', 'P', 'R', 'V');
	if (module->adjustableBindings() && isSpirV)
		return false;

	auto shaderData = module->shaders();
	if (!shaderData)
		return false;
	for (uint32_t i = 0; i < shaderData->size(); ++i)
	{
		if (!(*shaderData)[i])
			return false;
	}

	auto pipelines = module->pipelines();
	if (!pipelines)
		return false;
	for (uint32_t i = 0; i < pipelines->size(); ++i)
	{
		const mslb::Pipeline* pipeline = (*pipelines)[i];
		if (!pipeline)
			return false;

		if (!pipeline->name())
			return false;

		// Verify structs
		auto structs = pipeline->structs();
		if (!structs)
			return false;
		for (uint32_t j = 0; j < structs->size(); ++j)
		{
			const mslb::Struct* thisStruct = (*structs)[j];
			if (!thisStruct->name())
				return false;
			auto members = thisStruct->members();
			if (!members)
				return false;
			for (uint32_t k = 0; k < members->size(); ++k)
			{
				const mslb::StructMember* member = (*members)[k];
				if (!member)
					return false;
				if (!member->name())
					return false;
				if (!enumInRange(member->type()))
					return false;
				if (member->type() == mslb::Type::Struct &&
					member->structIndex() >= structs->size())
				{
					return false;
				}

				auto arrayElements = member->arrayElements();
				if (arrayElements)
				{
					for (uint32_t l = 0; l < arrayElements->size(); ++l)
					{
						if (!(*arrayElements)[l])
							return false;
					}
				}
			}
		}

		// Verify samplers
		auto samplerStates = pipeline->samplerStates();
		if (!samplerStates)
			return false;
		for (uint32_t j = 0; j < samplerStates->size(); ++j)
		{
			const mslb::SamplerState* sampler = (*samplerStates)[j];
			if (!sampler)
				return false;
			if (!enumInRange(sampler->minFilter()))
				return false;
			if (!enumInRange(sampler->magFilter()))
				return false;
			if (!enumInRange(sampler->mipFilter()))
				return false;
			if (!enumInRange(sampler->addressModeU()))
				return false;
			if (!enumInRange(sampler->addressModeV()))
				return false;
			if (!enumInRange(sampler->addressModeW()))
				return false;
			if (!enumInRange(sampler->borderColor()))
				return false;
		}

		// Verify uniforms
		auto uniforms = pipeline->uniforms();
		if (!uniforms)
			return false;
		for (uint32_t j = 0; j < uniforms->size(); ++j)
		{
			const mslb::Uniform* uniform = (*uniforms)[j];
			if (!uniform)
				return false;
			if (!uniform->name())
				return false;
			if (!enumInRange(uniform->uniformType()))
				return false;
			if (!enumInRange(uniform->type()))
				return false;
			if (uniform->type() == mslb::Type::Struct && uniform->structIndex() >= structs->size())
				return false;

			auto arrayElements = uniform->arrayElements();
			if (arrayElements)
			{
				for (uint32_t l = 0; l < arrayElements->size(); ++l)
				{
					if (!(*arrayElements)[l])
						return false;
				}
			}
		}

		// Verify attributes
		auto attributes = pipeline->attributes();
		if (!attributes)
			return false;
		for (uint32_t j = 0; j < attributes->size(); ++j)
		{
			const mslb::Attribute* attribute = (*attributes)[j];
			if (!attribute)
				return false;
			if (!attribute->name())
				return false;
			if (!enumInRange(attribute->type()) || attribute->type() == mslb::Type::Struct)
				return false;

			auto arrayElements = attribute->arrayElements();
			if (arrayElements)
			{
				for (uint32_t l = 0; l < arrayElements->size(); ++l)
				{
					if (!(*arrayElements)[l])
						return false;
				}
			}
		}

		// Verify push constant
		uint32_t pushConstantStruct = pipeline->pushConstantStruct();
		if (pushConstantStruct != MSL_UNKNOWN && pushConstantStruct >= structs->size())
			return false;

		// Verify render state
		const mslb::RenderState* renderState = pipeline->renderState();
		if (!renderState)
			return false;
		const mslb::RasterizationState* rasterizationState = renderState->rasterizationState();
		if (!rasterizationState)
			return false;
		if (!enumInRange(rasterizationState->depthClampEnable()))
			return false;
		if (!enumInRange(rasterizationState->rasterizerDiscardEnable()))
			return false;
		if (!enumInRange(rasterizationState->polygonMode()))
			return false;
		if (!enumInRange(rasterizationState->cullMode()))
			return false;
		if (!enumInRange(rasterizationState->frontFace()))
			return false;
		if (!enumInRange(rasterizationState->depthBiasEnable()))
			return false;

		const mslb::MultisampleState* multisampleState = renderState->multisampleState();
		if (!multisampleState)
			return false;
		if (!enumInRange(multisampleState->sampleShadingEnable()))
			return false;
		if (!enumInRange(multisampleState->alphaToCoverageEnable()))
			return false;
		if (!enumInRange(multisampleState->alphaToOneEnable()))
			return false;

		const mslb::DepthStencilState* depthStencilState = renderState->depthStencilState();
		if (!depthStencilState)
			return false;
		if (!enumInRange(depthStencilState->depthTestEnable()))
			return false;
		if (!enumInRange(depthStencilState->depthWriteEnable()))
			return false;
		if (!enumInRange(depthStencilState->depthCompareOp()))
			return false;
		if (!enumInRange(depthStencilState->depthBoundsTestEnable()))
			return false;
		if (!enumInRange(depthStencilState->stencilTestEnable()))
			return false;
		if (!isStencilOpStateValid(depthStencilState->frontStencil()))
			return false;
		if (!isStencilOpStateValid(depthStencilState->backStencil()))
			return false;

		const mslb::BlendState* blendState = renderState->blendState();
		if (!blendState)
			return false;
		if (!enumInRange(blendState->logicalOpEnable()))
			return false;
		if (!enumInRange(blendState->logicalOp()))
			return false;
		if (!enumInRange(blendState->separateAttachmentBlendingEnable()))
			return false;
		auto blendAttachments = blendState->blendAttachments();
		if (!blendAttachments || blendAttachments->size() != MSL_MAX_ATTACHMENTS)
			return false;
		for (uint32_t j = 0; j < MSL_MAX_ATTACHMENTS; ++j)
		{
			const mslb::BlendAttachmentState* attachment = (*blendAttachments)[j];
			if (!attachment)
				return false;
			if (!enumInRange(attachment->blendEnable()))
				return false;
			if (!enumInRange(attachment->srcColorBlendFactor()))
				return false;
			if (!enumInRange(attachment->dstColorBlendFactor()))
				return false;
			if (!enumInRange(attachment->colorBlendOp()))
				return false;
			if (!enumInRange(attachment->srcAlphaBlendFactor()))
				return false;
			if (!enumInRange(attachment->dstAlphaBlendFactor()))
				return false;
			if (!enumInRange(attachment->alphaBlendOp()))
				return false;
		}
		auto blendConstants = blendState->blendConstants();
		if (!blendConstants || blendConstants->size() != 4)
			return false;

		// Verify shaders
		auto shaders = pipeline->shaders();
		if (!shaders || shaders->size() != mslStage_Count)
			return false;
		for (int j = 0; j < mslStage_Count; ++j)
		{
			const mslb::Shader* shader = (*shaders)[i];
			if (shader || shader->shader() == MSL_UNKNOWN)
			{
				if (shader->shader() >= shaderData->size())
					return false;

				if (isSpirV)
				{
					auto uniformIds = shader->uniformIds();
					if (!uniformIds || uniformIds->size() != uniforms->size())
						return false;
				}
			}
		}
	}

	return true;
}

static void setUniformBinding(const flatbuffers::Vector<flatbuffers::Offset<mslb::Shader>>& shaders,
	mslSizedData shaderData[mslStage_Count], uint32_t uniformIndex, uint32_t descriptorSet,
	uint32_t binding)
{
	const unsigned int firstInstruction = 5;
	const uint32_t opCodeMask = 0xFFFF;
	const uint32_t wordCountShift = 16;
	const uint32_t opFunction = 54;
	const uint32_t opDecorate = 71;
	const uint32_t decorationBinding = 33;
	const uint32_t decorationDescriptorSet = 34;
	const uint32_t decorationInputAttachmentIndex = 43;
	for (int i = 0; i < mslStage_Count; ++i)
	{
		const mslb::Shader* shader = shaders[i];
		if (!shader || shader->shader() == MSL_UNKNOWN)
			continue;

		uint32_t id = (*shader->uniformIds())[uniformIndex];

		uint32_t* spirV = reinterpret_cast<uint32_t*>(shaderData[i].data);
		uint32_t spirVSize = static_cast<uint32_t>(shaderData[i].size/sizeof(uint32_t));
		for (uint32_t j = firstInstruction; j < spirVSize;)
		{
			uint32_t op = spirV[j] & opCodeMask;
			uint32_t wordCount = spirV[j] >> wordCountShift;

			// Once we reach the functions, done with all decorations.
			if (op == opFunction)
				break;

			if (op == opDecorate && spirV[j + 1] == id)
			{
				switch (spirV[j + 2])
				{
					case decorationBinding:
					case decorationInputAttachmentIndex:
						spirV[j + 3] = binding;
						break;
					case decorationDescriptorSet:
						spirV[j + 3] = descriptorSet;
						break;
				}
			}

			j += wordCount;
		}
	}
}

extern "C"
{

void mslModule_setInvalidFormatErrno(int errorCode)
{
	invalidFormatErrno = errorCode;
}

size_t mslModule_sizeof(size_t dataSize)
{
	return sizeof(mslModule) + dataSize;
}

mslModule* mslModule_readStream(mslReadFunction readFunc, void* userData,
	size_t size, const mslAllocator* allocator)
{
	if (!readFunc || size == 0 || (allocator && !allocator->allocateFunc))
	{
		errno = EINVAL;
		return nullptr;
	}

	mslModule* module = createModule(size, allocator);
	if (!module)
		return nullptr;

	size_t readSize = 0;
	do
	{
		size_t thisRead = readFunc(userData, module->data + readSize, size - readSize);
		if (thisRead == 0)
		{
			mslModule_destroy(module);
			errno = EIO;
			return nullptr;
		}

		readSize += thisRead;
	} while (readSize < size);

	if (!isValid(module->data, size))
	{
		mslModule_destroy(module);
		errno = invalidFormatErrno;
		return nullptr;
	}

	module->module = const_cast<mslb::Module*>(mslb::GetModule(module->data));
	fixupModule(module);
	return module;
}

mslModule* mslModule_readData(const void* data, size_t size, const mslAllocator* allocator)
{
	if (!data || size == 0 || !canUseAllocator(allocator))
	{
		errno = EINVAL;
		return nullptr;
	}

	if (!isValid(data, size))
	{
		errno = invalidFormatErrno;
		return nullptr;
	}

	mslModule* module = createModule(size, allocator);
	if (!module)
		return nullptr;

	memcpy(module->data, data, size);
	module->module = mslb::GetMutableModule(module->data);
	if (module->module->version() > MSL_MODULE_VERSION)
	{
		mslModule_destroy(module);
		errno = invalidFormatErrno;
		return nullptr;
	}

	module->module = const_cast<mslb::Module*>(mslb::GetModule(module->data));
	fixupModule(module);
	return module;
}

mslModule* mslModule_readFile(const char* fileName, const mslAllocator* allocator)
{
	if (!fileName || !canUseAllocator(allocator))
	{
		errno = EINVAL;
		return nullptr;
	}

	// Use FILE rather than fstream to make sure we don't have to link to the C++ library.
	FILE* file = fopen(fileName, "rb");
	if (!file)
		return nullptr;

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return nullptr;
	}

	size_t size = ftell(file);
	if (fseek(file, 0, SEEK_SET) != 0)
	{
		fclose(file);
		return nullptr;
	}

	mslModule* module = mslModule_readStream(&readFile, file, size, allocator);
	fclose(file);
	return module;
}

uint32_t mslModule_version(const mslModule* module)
{
	if (!module)
		return MSL_UNKNOWN;

	return module->module->version();
}

uint32_t mslModule_targetId(const mslModule* module)
{
	if (!module)
		return MSL_UNKNOWN;

	return module->module->targetId();
}

uint32_t mslModule_targetVersion(const mslModule* module)
{
	if (!module)
		return MSL_UNKNOWN;

	return module->module->targetVersion();
}

bool mslModule_adjustableBindings(const mslModule* module)
{
	if (!module)
		return false;

	return module->module->adjustableBindings();
}

uint32_t mslModule_pipelineCount(const mslModule* module)
{
	if (!module)
		return MSL_UNKNOWN;

	return module->module->pipelines()->size();
}

bool mslModule_pipeline(mslPipeline* outPipeline, const mslModule* module,
	uint32_t pipelineIndex)
{
	if (!outPipeline || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;

	const mslb::Pipeline* pipeline = pipelines[pipelineIndex];
	outPipeline->name = pipeline->name()->c_str();
	outPipeline->structCount = pipeline->structs()->size();
	outPipeline->samplerStateCount = pipeline->samplerStates()->size();
	outPipeline->uniformCount = pipeline->uniforms()->size();
	outPipeline->attributeCount = pipeline->attributes()->size();
	outPipeline->pushConstantStruct = pipeline->pushConstantStruct();

	auto& shaders = *pipeline->shaders();
	for (int i = 0; i < mslStage_Count; ++i)
		outPipeline->shaders[i] = shaders[i] ? shaders[i]->shader() : MSL_UNKNOWN;

	return true;
}

bool mslModule_struct(mslStruct* outStruct, const mslModule* module, uint32_t pipelineIndex,
	uint32_t structIndex)
{
	if (!outStruct || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;
	auto& structs = *pipelines[pipelineIndex]->structs();
	if (structIndex >= structs.size())
		return false;

	const mslb::Struct* pipelineStruct = structs[structIndex];
	outStruct->name = pipelineStruct->name()->c_str();
	outStruct->size = pipelineStruct->size();
	outStruct->memberCount = pipelineStruct->members()->size();
	return true;
}

bool mslModule_structMember(mslStructMember* outStructMember, const mslModule* module,
	uint32_t pipelineIndex, uint32_t structIndex, uint32_t structMemberIndex)
{
	if (!outStructMember || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;
	auto& structs = *pipelines[pipelineIndex]->structs();
	if (structIndex >= structs.size())
		return false;
	auto& members = *structs[structIndex]->members();
	if (structMemberIndex >= members.size())
		return false;

	const mslb::StructMember* member = members[structMemberIndex];
	outStructMember->name = member->name()->c_str();
	outStructMember->offset = member->offset();
	outStructMember->size = member->size();
	outStructMember->type = static_cast<mslType>(member->type());
	outStructMember->structIndex = member->structIndex();
	auto arrayElements = member->arrayElements();
	outStructMember->arrayElementCount = arrayElements ? arrayElements->size() : 0;
	outStructMember->rowMajor = member->rowMajor();
	return true;
}

bool mslModule_structMemberArrayInfo(mslArrayInfo* outArrayInfo,
	const mslModule* module, uint32_t pipelineIndex, uint32_t structIndex,
	uint32_t structMemberIndex, uint32_t arrayElement)
{
	if (!outArrayInfo || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;
	auto& structs = *pipelines[pipelineIndex]->structs();
	if (structIndex >= structs.size())
		return false;
	auto& members = *structs[structIndex]->members();
	if (structMemberIndex >= members.size())
		return false;
	auto arrayElements = members[structMemberIndex]->arrayElements();
	if (!arrayElements || arrayElement >= arrayElements->size())
		return false;

	const mslb::ArrayInfo* arrayInfo = (*arrayElements)[arrayElement];
	outArrayInfo->length = arrayInfo->length();
	outArrayInfo->stride = arrayInfo->stride();
	return true;
}

bool mslModule_samplerState(mslSamplerState* outSamplerState, const mslModule* module,
	uint32_t pipelineIndex, uint32_t samplerStateIndex)
{
	if (!outSamplerState || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;
	auto& samplerStates = *pipelines[pipelineIndex]->samplerStates();
	if (samplerStateIndex >= samplerStates.size())
		return false;

	const mslb::SamplerState* samplerState = samplerStates[samplerStateIndex];
	outSamplerState->minFilter = static_cast<mslFilter>(samplerState->minFilter());
	outSamplerState->magFilter = static_cast<mslFilter>(samplerState->magFilter());
	outSamplerState->mipFilter = static_cast<mslMipFilter>(samplerState->mipFilter());
	outSamplerState->addressModeU = static_cast<mslAddressMode>(samplerState->addressModeU());
	outSamplerState->addressModeV = static_cast<mslAddressMode>(samplerState->addressModeV());
	outSamplerState->addressModeW = static_cast<mslAddressMode>(samplerState->addressModeW());
	outSamplerState->mipLodBias = samplerState->mipLodBias();
	outSamplerState->maxAnisotropy = samplerState->maxAnisotropy();
	outSamplerState->minLod = samplerState->minLod();
	outSamplerState->maxLod = samplerState->maxLod();
	outSamplerState->borderColor = static_cast<mslBorderColor>(samplerState->borderColor());
	return true;
}

bool mslModule_uniform(mslUniform* outUniform, const mslModule* module, uint32_t pipelineIndex,
	uint32_t uniformIndex)
{
	if (!outUniform || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;
	auto& uniforms = *pipelines[pipelineIndex]->uniforms();
	if (uniformIndex >= uniforms.size())
		return false;

	const mslb::Uniform* uniform = uniforms[uniformIndex];
	outUniform->name = uniform->name()->c_str();
	outUniform->uniformType = static_cast<mslUniformType>(uniform->uniformType());
	outUniform->type = static_cast<mslType>(uniform->type());
	outUniform->structIndex = uniform->structIndex();
	auto arrayElements = uniform->arrayElements();
	outUniform->arrayElementCount = arrayElements ? arrayElements->size() : 0;
	outUniform->descriptorSet = uniform->descriptorSet();
	outUniform->binding = uniform->binding();
	outUniform->samplerIndex = uniform->samplerIndex();
	return true;
}

bool mslModule_uniformArrayInfo(mslArrayInfo* outArrayInfo, const mslModule* module,
	uint32_t pipelineIndex, uint32_t uniformIndex, uint32_t arrayElement)
{
	if (!outArrayInfo || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;
	auto& uniforms = *pipelines[pipelineIndex]->uniforms();
	if (uniformIndex >= uniforms.size())
		return false;
	auto arrayElements = uniforms[uniformIndex]->arrayElements();
	if (!arrayElements || arrayElement >= arrayElements->size())
		return false;

	const mslb::ArrayInfo* arrayInfo = (*arrayElements)[arrayElement];
	outArrayInfo->length = arrayInfo->length();
	outArrayInfo->stride = arrayInfo->stride();
	return true;
}

bool mslModule_attribute(mslAttribute* outAttribute, const mslModule* module,
	uint32_t pipelineIndex, uint32_t attributeIndex)
{
	if (!outAttribute || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;
	auto& attributes = *pipelines[pipelineIndex]->attributes();
	if (attributeIndex >= attributes.size())
		return false;

	const mslb::Attribute* attribute = attributes[attributeIndex];
	outAttribute->name = attribute->name()->c_str();
	outAttribute->type = static_cast<mslType>(attribute->type());
	auto arrayElements = attribute->arrayElements();
	outAttribute->arrayElementCount = arrayElements ? arrayElements->size() : 0;
	outAttribute->location = attribute->location();
	outAttribute->component = attribute->component();
	return true;
}

uint32_t mslModule_attributeArrayLength(const mslModule* module, uint32_t pipelineIndex,
	uint32_t attributeIndex, uint32_t arrayElement)
{
	if (!module)
		return MSL_UNKNOWN;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return MSL_UNKNOWN;
	auto& attributes = *pipelines[pipelineIndex]->attributes();
	if (attributeIndex >= attributes.size())
		return MSL_UNKNOWN;
	auto arrayElements = attributes[attributeIndex]->arrayElements();
	if (!arrayElements || arrayElement >= arrayElements->size())
		return MSL_UNKNOWN;

	return (*arrayElements)[arrayElement];
}

bool mslModule_renderState(mslRenderState* outRenderState, const mslModule* module,
	uint32_t pipelineIndex)
{
	if (!outRenderState || !module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;

	const mslb::RenderState* renderState = pipelines[pipelineIndex]->renderState();

	const mslb::RasterizationState* rasterizationState = renderState->rasterizationState();
	outRenderState->rasterizationState.depthClampEnable =
		static_cast<mslBool>(rasterizationState->depthClampEnable());
	outRenderState->rasterizationState.rasterizerDiscardEnable =
		static_cast<mslBool>(rasterizationState->rasterizerDiscardEnable());
	outRenderState->rasterizationState.polygonMode =
		static_cast<mslPolygonMode>(rasterizationState->polygonMode());
	outRenderState->rasterizationState.cullMode =
		static_cast<mslCullMode>(rasterizationState->cullMode());
	outRenderState->rasterizationState.frontFace =
		static_cast<mslFrontFace>(rasterizationState->frontFace());
	outRenderState->rasterizationState.depthBiasEnable =
		static_cast<mslBool>(rasterizationState->depthBiasEnable());
	outRenderState->rasterizationState.depthBiasConstantFactor =
		rasterizationState->depthBiasConstantFactor();
	outRenderState->rasterizationState.depthBiasClamp = rasterizationState->depthBiasClamp();
	outRenderState->rasterizationState.depthBiasSlopeFactor =
		rasterizationState->depthBiasSlopeFactor();
	outRenderState->rasterizationState.lineWidth = rasterizationState->lineWidth();

	const mslb::MultisampleState* multisampleState = renderState->multisampleState();
	outRenderState->multisampleState.sampleShadingEnable =
		static_cast<mslBool>(multisampleState->sampleShadingEnable());
	outRenderState->multisampleState.minSampleShading = multisampleState->minSampleShading();
	outRenderState->multisampleState.sampleMask = multisampleState->sampleMask();
	outRenderState->multisampleState.alphaToCoverageEnable =
		static_cast<mslBool>(multisampleState->alphaToCoverageEnable());
	outRenderState->multisampleState.alphaToOneEnable =
		static_cast<mslBool>(multisampleState->alphaToOneEnable());

	const mslb::DepthStencilState* depthStencilState = renderState->depthStencilState();
	outRenderState->depthStencilState.depthTestEnable =
		static_cast<mslBool>(depthStencilState->depthTestEnable());
	outRenderState->depthStencilState.depthWriteEnable =
		static_cast<mslBool>(depthStencilState->depthWriteEnable());
	outRenderState->depthStencilState.depthCompareOp =
		static_cast<mslCompareOp>(depthStencilState->depthCompareOp());
	outRenderState->depthStencilState.depthBoundsTestEnable =
		static_cast<mslBool>(depthStencilState->depthBoundsTestEnable());
	outRenderState->depthStencilState.stencilTestEnable =
		static_cast<mslBool>(depthStencilState->stencilTestEnable());

	outRenderState->depthStencilState.frontStencil.failOp =
		static_cast<mslStencilOp>(depthStencilState->frontStencil().failOp());
	outRenderState->depthStencilState.frontStencil.passOp =
		static_cast<mslStencilOp>(depthStencilState->frontStencil().passOp());
	outRenderState->depthStencilState.frontStencil.depthFailOp =
		static_cast<mslStencilOp>(depthStencilState->frontStencil().depthFailOp());
	outRenderState->depthStencilState.frontStencil.compareOp =
		static_cast<mslCompareOp>(depthStencilState->frontStencil().compareOp());
	outRenderState->depthStencilState.frontStencil.compareMask =
		depthStencilState->frontStencil().compareMask();
	outRenderState->depthStencilState.frontStencil.writeMask =
		depthStencilState->frontStencil().writeMask();
	outRenderState->depthStencilState.frontStencil.reference =
		depthStencilState->frontStencil().reference();

	outRenderState->depthStencilState.backStencil.failOp =
		static_cast<mslStencilOp>(depthStencilState->backStencil().failOp());
	outRenderState->depthStencilState.backStencil.passOp =
		static_cast<mslStencilOp>(depthStencilState->backStencil().passOp());
	outRenderState->depthStencilState.backStencil.depthFailOp =
		static_cast<mslStencilOp>(depthStencilState->backStencil().depthFailOp());
	outRenderState->depthStencilState.backStencil.compareOp =
		static_cast<mslCompareOp>(depthStencilState->backStencil().compareOp());
	outRenderState->depthStencilState.backStencil.compareMask =
		depthStencilState->backStencil().compareMask();
	outRenderState->depthStencilState.backStencil.writeMask =
		depthStencilState->backStencil().writeMask();
	outRenderState->depthStencilState.backStencil.reference =
		depthStencilState->backStencil().reference();

	outRenderState->depthStencilState.minDepthBounds = depthStencilState->minDepthBounds();
	outRenderState->depthStencilState.maxDepthBounds = depthStencilState->maxDepthBounds();

	const mslb::BlendState* blendState = renderState->blendState();
	outRenderState->blendState.logicalOpEnable =
		static_cast<mslBool>(blendState->logicalOpEnable());
	outRenderState->blendState.logicalOp = static_cast<mslLogicOp>(blendState->logicalOp());
	outRenderState->blendState.separateAttachmentBlendingEnable =
		static_cast<mslBool>(blendState->separateAttachmentBlendingEnable());
	auto& blendAttachments = *blendState->blendAttachments();
	for (unsigned int i = 0; i < MSL_MAX_ATTACHMENTS; ++i)
	{
		const mslb::BlendAttachmentState* blendAttachmentState = blendAttachments[i];
		outRenderState->blendState.blendAttachments[i].blendEnable =
			static_cast<mslBool>(blendAttachmentState->blendEnable());
		outRenderState->blendState.blendAttachments[i].srcColorBlendFactor =
			static_cast<mslBlendFactor>(blendAttachmentState->srcColorBlendFactor());
		outRenderState->blendState.blendAttachments[i].dstColorBlendFactor =
			static_cast<mslBlendFactor>(blendAttachmentState->dstColorBlendFactor());
		outRenderState->blendState.blendAttachments[i].colorBlendOp =
			static_cast<mslBlendOp>(blendAttachmentState->colorBlendOp());
		outRenderState->blendState.blendAttachments[i].srcAlphaBlendFactor =
			static_cast<mslBlendFactor>(blendAttachmentState->srcAlphaBlendFactor());
		outRenderState->blendState.blendAttachments[i].dstAlphaBlendFactor =
			static_cast<mslBlendFactor>(blendAttachmentState->dstAlphaBlendFactor());
		outRenderState->blendState.blendAttachments[i].alphaBlendOp =
			static_cast<mslBlendOp>(blendAttachmentState->alphaBlendOp());
		outRenderState->blendState.blendAttachments[i].colorWriteMask =
			static_cast<mslColorMask>(blendAttachmentState->colorWriteMask());
	}
	auto& blendConstants = *blendState->blendConstants();
	for (unsigned int i = 0; i < 4; ++i)
		outRenderState->blendState.blendConstants[i] = blendConstants[i];

	outRenderState->patchControlPoints = renderState->patchControlPoints();
	return true;
}

bool mslModule_setUniformBinding(mslModule* module, uint32_t pipelineIndex, uint32_t uniformIndex,
	uint32_t descriptorSet, uint32_t binding)
{
	if (!module || !module->module->adjustableBindings())
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;

	const mslb::Pipeline* pipeline = pipelines[pipelineIndex];
	auto& uniforms = *pipeline->uniforms();
	if (uniformIndex >= uniforms.size())
		return false;
	mslb::Uniform* uniform = const_cast<mslb::Uniform*>(uniforms[uniformIndex]);

	// Set the new indices.
	uniform->mutate_descriptorSet(descriptorSet);
	uniform->mutate_binding(binding);

	// Modify the SPIR-V.
	mslSizedData shaderDataArray[mslStage_Count] = {};
	auto& shaderData = *module->module->shaders();
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (shaderData[i] && shaderData[i]->data())
		{
			shaderDataArray[i].data =
				const_cast<uint8_t*>(shaderData[i]->data()->data());
			shaderDataArray[i].size = shaderData[i]->data()->size();
		}
	}
	setUniformBinding(*pipeline->shaders(), shaderDataArray, uniformIndex, descriptorSet, binding);
	return true;
}

bool mslModule_setUniformBindingCopy(const mslModule* module, uint32_t pipelineIndex,
	uint32_t uniformIndex, uint32_t descriptorSet, uint32_t binding,
	mslSizedData shaderData[mslStage_Count])
{
	if (!module)
		return false;

	auto& pipelines = *module->module->pipelines();
	if (pipelineIndex >= pipelines.size())
		return false;

	const mslb::Pipeline* pipeline = pipelines[pipelineIndex];
	auto& uniforms = *pipeline->uniforms();
	if (uniformIndex >= uniforms.size())
		return false;

	// Modify the SPIR-V.
	mslSizedData shaderDataArray[mslStage_Count] = {};
	const auto& expectedShaderData = *module->module->shaders();
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (expectedShaderData[i] && expectedShaderData[i]->data())
		{
			if (shaderData[i].size != expectedShaderData[i]->data()->size())
				return false;
		}
		else
		{
			if (shaderData[i].size > 0)
				return false;
		}
	}
	setUniformBinding(*pipeline->shaders(), shaderDataArray, uniformIndex, descriptorSet, binding);
	return true;
}

uint32_t mslModule_shaderCount(const mslModule* module)
{
	if (!module)
		return 0;

	return module->module->shaders()->size();
}

uint32_t mslModule_shaderSize(const mslModule* module, uint32_t shader)
{
	if (!module)
		return 0;

	auto& shaders = *module->module->shaders();
	if (shader >= shaders.size())
		return 0;

	return shaders[shader]->data()->size();
}

const void* mslModule_shaderData(const mslModule* module, uint32_t shader)
{
	if (!module)
		return nullptr;

	auto& shaders = *module->module->shaders();
	if (shader >= shaders.size())
		return nullptr;

	return shaders[shader]->data()->data();
}

uint32_t mslModule_sharedDataSize(const mslModule* module)
{
	if (!module)
		return 0;

	return module->module->sharedData()->size();
}

const void* mslModule_sharedData(const mslModule* module)
{
	if (!module)
		return nullptr;

	return module->module->sharedData()->data();
}

void mslModule_destroy(mslModule* module)
{
	if (!module || (module->allocator.allocateFunc && !module->allocator.freeFunc))
		return;

	if (module->allocator.freeFunc)
		module->allocator.freeFunc(module->allocator.userData, module);
	else
		free(module);
}

} // extern "C"
