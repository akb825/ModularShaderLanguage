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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static_assert(static_cast<unsigned int>(mslb::Type::MAX) == mslType_Count - 1,
	"Type enum mismatch between flatbuffer and mslType.");

extern "C"
{

struct mslModule
{
	mslAllocator allocator;
	const mslb::Module* module;
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

size_t mslModule_sizeof(size_t dataSize)
{
	return sizeof(mslModule) + dataSize;
}

mslModule* mslModule_readStream(mslReadFunction readFunc, void* userData,
	size_t size, const mslAllocator* allocator)
{
	if (!readFunc || size == 0 || (allocator && !allocator->allocateFunc))
		return nullptr;

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
			return nullptr;
		}

		readSize += thisRead;
	} while (readSize < size);

	flatbuffers::Verifier verifier(module->data, size);
	if (!mslb::VerifyModuleBuffer(verifier))
	{
		mslModule_destroy(module);
		return nullptr;
	}

	module->module = mslb::GetModule(module->data);
	if (module->module->version() > MSL_MODULE_VERSION)
	{
		mslModule_destroy(module);
		return nullptr;
	}

	fixupModule(module);
	return module;
}

mslModule* mslModule_readData(const void* data, size_t size, const mslAllocator* allocator)
{
	if (!data || size == 0 || !canUseAllocator(allocator))
		return nullptr;

	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!mslb::VerifyModuleBuffer(verifier))
		return nullptr;

	mslModule* module = createModule(size, allocator);
	if (!module)
		return nullptr;

	memcpy(module->data, data, size);
	module->module = mslb::GetModule(module->data);
	if (module->module->version() > MSL_MODULE_VERSION)
	{
		mslModule_destroy(module);
		return nullptr;
	}

	fixupModule(module);
	return module;
}

mslModule* mslModule_readFile(const char* fileName, const mslAllocator* allocator)
{
	if (!fileName || !canUseAllocator(allocator))
		return nullptr;

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

uint32_t mslModule_pipelineCount(const mslModule* module)
{
	if (!module)
		return MSL_UNKNOWN;

	return module->module->pipelines()->size();
}

const char* mslModule_pipelineName(const mslModule* module, uint32_t pipeline)
{
	if (!module)
		return nullptr;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return nullptr;

	return pipelines[pipeline]->name()->c_str();
}

uint32_t mslModule_pipelineShader(const mslModule* module, uint32_t pipeline, mslStage stage)
{
	if (!module)
		return MSL_NO_SHADER;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return MSL_NO_SHADER;

	switch (stage)
	{
		case mslStage_Vertex:
			return pipelines[pipeline]->vertex();
		case mslStage_TessellationControl:
			return pipelines[pipeline]->tessellationControl();
		case mslStage_TessellationEvaluation:
			return pipelines[pipeline]->tessellationEvaluation();
		case mslStage_Geometry:
			return pipelines[pipeline]->geometry();
		case mslStage_Fragment:
			return pipelines[pipeline]->fragment();
		case mslStage_Compute:
			return pipelines[pipeline]->compute();
		default:
			return MSL_NO_SHADER;
	}
}

uint32_t mslModule_uniformCount(const mslModule* module, uint32_t pipeline)
{
	if (!module)
		return 0;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return 0;

	return pipelines[pipeline]->uniforms()->size();
}

const char* mslModule_uniformName(const mslModule* module, uint32_t pipeline, uint32_t uniform)
{
	if (!module)
		return nullptr;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return nullptr;

	auto& uniforms = *pipelines[pipeline]->uniforms();
	if (uniform >= uniforms.size())
		return nullptr;

	return uniforms[uniform]->name()->c_str();
}

mslType mslModule_uniformType(const mslModule* module, uint32_t pipeline, uint32_t uniform)
{
	if (!module)
		return mslType_Float;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return mslType_Float;

	auto& uniforms = *pipelines[pipeline]->uniforms();
	if (uniform >= uniforms.size())
		return mslType_Float;

	return static_cast<mslType>(uniforms[uniform]->type());
}

uint32_t mslModule_uniformBlockIndex(const mslModule* module, uint32_t pipeline,
	uint32_t uniform)
{
	if (!module)
		return MSL_UNKNOWN;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return MSL_UNKNOWN;

	auto& uniforms = *pipelines[pipeline]->uniforms();
	if (uniform >= uniforms.size())
		return MSL_UNKNOWN;

	return uniforms[uniform]->blockIndex();
}

uint32_t mslModule_uniformBufferOffset(const mslModule* module, uint32_t pipeline, uint32_t uniform)
{
	if (!module)
		return MSL_UNKNOWN;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return MSL_UNKNOWN;

	auto& uniforms = *pipelines[pipeline]->uniforms();
	if (uniform >= uniforms.size())
		return MSL_UNKNOWN;

	return uniforms[uniform]->bufferOffset();
}

uint32_t mslModule_uniformElements(const mslModule* module, uint32_t pipeline, uint32_t uniform)
{
	if (!module)
		return 0;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return 0;

	auto& uniforms = *pipelines[pipeline]->uniforms();
	if (uniform >= uniforms.size())
		return 0;

	return uniforms[uniform]->elements();
}

uint32_t mslModule_uniformBlockCount(const mslModule* module, uint32_t pipeline)
{
	if (!module)
		return 0;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return 0;

	return pipelines[pipeline]->uniformBlocks()->size();
}

const char* mslModule_uniformBlockName(const mslModule* module, uint32_t pipeline, uint32_t block)
{
	if (!module)
		return nullptr;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return nullptr;

	auto& uniformBlocks = *pipelines[pipeline]->uniformBlocks();
	if (block >= uniformBlocks.size())
		return nullptr;

	return uniformBlocks[block]->name()->c_str();
}

uint32_t mslModule_uniformBlockSize(const mslModule* module, uint32_t pipeline, uint32_t block)
{
	if (!module)
		return MSL_UNKNOWN;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return MSL_UNKNOWN;

	auto& uniformBlocks = *pipelines[pipeline]->uniformBlocks();
	if (block >= uniformBlocks.size())
		return MSL_UNKNOWN;

	return uniformBlocks[block]->size();
}

uint32_t mslModule_attributeCount(const mslModule* module, uint32_t pipeline)
{
	if (!module)
		return 0;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return 0;

	return pipelines[pipeline]->attributes()->size();
}

const char* mslModule_attributeName(const mslModule* module, uint32_t pipeline, uint32_t attribute)
{
	if (!module)
		return nullptr;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return nullptr;

	auto& attributes = *pipelines[pipeline]->attributes();
	if (attribute >= attributes.size())
		return nullptr;

	return attributes[attribute]->name()->c_str();
}

mslType mslModule_attributeType(const mslModule* module, uint32_t pipeline, uint32_t attribute)
{
	if (!module)
		return mslType_Float;

	auto& pipelines = *module->module->pipelines();
	if (pipeline >= pipelines.size())
		return mslType_Float;

	auto& attributes = *pipelines[pipeline]->attributes();
	if (attribute >= attributes.size())
		return mslType_Float;

	return static_cast<mslType>(attributes[attribute]->type());
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
