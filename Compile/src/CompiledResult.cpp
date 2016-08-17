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
#include "mslb_generated.h"
#include <fstream>

namespace msl
{

static_assert(static_cast<unsigned int>(mslb::Type::MAX) == CompiledResult::typeCount - 1,
	"Type enum mismatch between flatbuffer and CompiledResult.");

CompiledResult::CompiledResult()
	: m_target(nullptr)
	, m_targetId(0)
	, m_targetVersion(0)
{
}

std::size_t CompiledResult::addShader(std::vector<uint8_t> shader)
{
	for (std::size_t i = 0; i < m_shaders.size(); ++i)
	{
		if (m_shaders[i] == shader)
			return i;
	}

	m_shaders.push_back(std::move(shader));
	return m_shaders.size() - 1;
}

bool CompiledResult::save(std::ostream& stream) const
{
	if (!m_target)
		return false;

	flatbuffers::FlatBufferBuilder builder;
	std::vector<flatbuffers::Offset<mslb::Pipeline>> pipelines(m_pipelines.size());
	std::vector<flatbuffers::Offset<mslb::Uniform>> uniforms;
	std::vector<flatbuffers::Offset<mslb::UniformBlock>> uniformBlocks;
	std::vector<flatbuffers::Offset<mslb::Attribute>> attributes;
	std::size_t i = 0;
	for (const auto& pipeline : m_pipelines)
	{
		uniforms.resize(pipeline.second.uniforms.size());
		for (std::size_t j = 0; j < uniforms.size(); ++j)
		{
			uniforms[j] = mslb::CreateUniform(builder,
				builder.CreateString(pipeline.second.uniforms[j].name),
				static_cast<mslb::Type>(pipeline.second.uniforms[j].type),
				pipeline.second.uniforms[j].blockIndex,
				pipeline.second.uniforms[j].bufferOffset,
				pipeline.second.uniforms[j].elements);
		}

		uniformBlocks.resize(pipeline.second.uniformBlocks.size());
		for (std::size_t j = 0; j < uniformBlocks.size(); ++j)
		{
			uniformBlocks[j] = mslb::CreateUniformBlock(builder,
				builder.CreateString(pipeline.second.uniformBlocks[j].name),
				pipeline.second.uniformBlocks[j].size);
		}

		attributes.resize(pipeline.second.attributes.size());
		for (std::size_t j = 0; j < attributes.size(); ++j)
		{
			attributes[j] = mslb::CreateAttribute(builder,
				builder.CreateString(pipeline.second.attributes[j].name),
				static_cast<mslb::Type>(pipeline.second.attributes[j].type));
		}

		pipelines[i] = mslb::CreatePipeline(builder,
			builder.CreateString(pipeline.first),
			static_cast<std::uint32_t>(pipeline.second.vertex),
			static_cast<std::uint32_t>(pipeline.second.tessellationControl),
			static_cast<std::uint32_t>(pipeline.second.tessellationEvaluation),
			static_cast<std::uint32_t>(pipeline.second.geometry),
			static_cast<std::uint32_t>(pipeline.second.fragment),
			static_cast<std::uint32_t>(pipeline.second.compute),
			builder.CreateVector(uniforms),
			builder.CreateVector(uniformBlocks),
			builder.CreateVector(attributes));
		++i;
	}

	// Swap if big endian and SPIR-V since it uses 32-bit values.
	bool swap = !FLATBUFFERS_LITTLEENDIAN && m_targetId == MSL_CREATE_ID('S', 'P', 'R', 'V');
	std::vector<uint8_t> swapShader;
	std::vector<flatbuffers::Offset<mslb::Shader>> shaders(m_shaders.size());
	for (i = 0; i < m_shaders.size(); ++i)
	{
		if (swap)
		{
			swapShader = m_shaders[i];
			std::uint32_t* swapShader32 = reinterpret_cast<std::uint32_t*>(swapShader.data());
			std::size_t swapShader32Size = swapShader.size()/sizeof(std::uint32_t);
			for (std::size_t j = 0; j < swapShader32Size; ++j)
				swapShader32[j] = flatbuffers::EndianScalar(swapShader[j]);
			shaders[i] = mslb::CreateShader(builder, builder.CreateVector(swapShader));
		}
		else
			shaders[i] = mslb::CreateShader(builder, builder.CreateVector(m_shaders[i]));
	}

	builder.Finish(mslb::CreateModule(builder,
		version,
		m_targetId,
		m_targetVersion,
		builder.CreateVector(pipelines),
		builder.CreateVector(shaders),
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
