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

#include <MSL/Compile/TargetGlsl.h>
#include <MSL/Compile/Output.h>
#include "ExecuteCommand.h"
#include "GlslOutput.h"

namespace msl
{

TargetGlsl::TargetGlsl(std::uint32_t version, bool isEs)
	: m_version(version)
	, m_es(isEs)
	, m_remapDepthRange(true)
	, m_defaultFloatPrecision(Precision::Medium)
	, m_defaultIntPrecision(Precision::High)
{
}

bool TargetGlsl::isEs() const
{
	return m_es;
}

bool TargetGlsl::getRemapDepthRange() const
{
	return m_remapDepthRange;
}

void TargetGlsl::setRemapDepthRange(bool remap)
{
	m_remapDepthRange = remap;
}

TargetGlsl::Precision TargetGlsl::getDefaultFloatPrecision() const
{
	return m_defaultFloatPrecision;
}

void TargetGlsl::setDefaultFloatPrecision(Precision precision)
{
	m_defaultFloatPrecision = precision;
}

TargetGlsl::Precision TargetGlsl::getDefaultIntPrecision() const
{
	return m_defaultIntPrecision;
}

void TargetGlsl::setDefaultIntPrecision(Precision precision)
{
	m_defaultIntPrecision = precision;
}

void TargetGlsl::addHeaderLine(std::string header)
{
	m_headerLines.push_back(std::move(header));
}

const std::vector<std::string>& TargetGlsl::getHeaderLines() const
{
	return m_headerLines;
}

void TargetGlsl::clearHeaderLines()
{
	m_headerLines.clear();
}

void TargetGlsl::addRequiredExtension(std::string extension)
{
	m_requiredExtensions.push_back(std::move(extension));
}

const std::vector<std::string>& TargetGlsl::getRequiredExtensions() const
{
	return m_requiredExtensions;
}

void TargetGlsl::clearRequiredExtensions()
{
	m_requiredExtensions.clear();
}

std::uint32_t TargetGlsl::getId() const
{
	if (m_es)
		return MSL_CREATE_ID('G', 'L', 'E', 'S');
	else
		return MSL_CREATE_ID('G', 'L', 'S', 'L');
}

std::uint32_t TargetGlsl::getVersion() const
{
	return m_version;
}

bool TargetGlsl::featureSupported(Feature feature) const
{
	switch (feature)
	{
		case Feature::Integers:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 130;
		case Feature::Doubles:
			if (m_es)
				return false;
			else
				return m_version >= 400;
		case Feature::NonSquareMatrices:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 120;
		case Feature::Texture3D:
			if (m_es)
				return m_version >= 300;
			else
				return true;
		case Feature::TextureArray:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 130;
		case Feature::ShadowSamplers:
			if (m_es)
				return m_version >= 300;
			else
				return true;
		case Feature::MultisampledTextures:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 150;
		case Feature::IntegerTextures:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 130;
		case Feature::Images:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 130;
		case Feature::UniformBuffers:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 420;
		case Feature::Buffers:
			if (m_es)
				return m_version >= 320;
			else
				return m_version >= 430;
		case Feature::Std140:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 150;
		case Feature::Std430:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 430;
		case Feature::TessellationStages:
			if (m_es)
				return m_version >= 320;
			else
				return m_version >= 400;
		case Feature::GeometryStage:
			if (m_es)
				return m_version >= 320;
			else
				return m_version >= 400;
		case Feature::ComputeStage:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 430;
		case Feature::MultipleRenderTargets:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 110;
		case Feature::DualSourceBlending:
			if (m_es)
				return false;
			else
				return m_version >= 150;
		case Feature::DepthHints:
			if (m_es)
				return false;
			else
				return m_version >= 420;
		case Feature::Derivatives:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 110;
		case Feature::AdvancedDerivatives:
			if (m_es)
				return false;
			else
				return m_version >= 450;
		case Feature::MemoryBarriers:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 400;
		case Feature::PrimitiveStreams:
			if (m_es)
				return m_version >= 320;
			else
				return m_version >= 400;
		case Feature::InterpolationFunctions:
			if (m_es)
				return false;
			else
				return m_version >= 400;
		case Feature::TextureGather:
			if (m_es)
				return m_version >= 320;
			else
				return m_version >= 400;
		case Feature::TexelFetch:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 130;
		case Feature::TextureSize:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 130;
		case Feature::TextureQueryLod:
			if (m_es)
				return false;
			else
				return m_version >= 400;
		case Feature::TextureQueryLevels:
			if (m_es)
				return false;
			else
				return m_version >= 400;
		case Feature::TextureSamples:
			if (m_es)
				return false;
			else
				return m_version >= 450;
		case Feature::BitFunctions:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 400;
		case Feature::PackingFunctions:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 410;
	}

	return false;
}

std::vector<std::uint8_t> TargetGlsl::crossCompile(Output& output,
	const std::vector<std::uint32_t>& spirv, const std::string& fileName, std::size_t line,
	std::size_t column) const
{
	GlslOutput::Options options;
	options.version = m_version;
	options.es = m_es;
	options.remapDepthRange = m_remapDepthRange;
	options.defaultFloatPrecision = m_defaultFloatPrecision;
	options.defaultIntPrecision = m_defaultIntPrecision;
	options.headerLines = m_headerLines;
	options.requiredExtensions = m_requiredExtensions;
	std::string glsl = GlslOutput::disassemble(output, spirv, options, fileName, line, column);
	std::vector<std::uint8_t> glslData(glsl.begin(), glsl.end());

	// Use external command if set.
	if (!m_glslToolCommand.empty())
	{
		ExecuteCommand command;
		command.getInput().write(reinterpret_cast<const char*>(glslData.data()), glslData.size());
		if (!command.execute(output, m_glslToolCommand))
			return std::vector<std::uint8_t>();

		glslData.assign(std::istreambuf_iterator<char>(command.getOutput().rdbuf()),
			std::istreambuf_iterator<char>());
	}

	return glslData;
}

} // namespace msl
