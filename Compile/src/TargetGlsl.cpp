/*
 * Copyright 2016-2022 Aaron Barany
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
#include <sstream>

namespace msl
{

using namespace compile;

TargetGlsl::TargetGlsl(std::uint32_t version, bool isEs)
	: m_version(version)
	, m_es(isEs)
	, m_remapDepthRange(false)
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

void TargetGlsl::addHeaderLine(const std::string& header)
{
	for (auto& headerLines : m_headerLines)
		headerLines.push_back(header);
}

void TargetGlsl::addHeaderLine(Stage stage, std::string header)
{
	m_headerLines[static_cast<std::size_t>(stage)].push_back(std::move(header));
}

const std::vector<std::string>& TargetGlsl::getHeaderLines(Stage stage) const
{
	return m_headerLines[static_cast<std::size_t>(stage)];
}

void TargetGlsl::clearHeaderLines()
{
	for (auto& headerLines : m_headerLines)
		headerLines.clear();
}

void TargetGlsl::addRequiredExtension(const std::string& extension)
{
	for (auto& requiredExtensions : m_requiredExtensions)
		requiredExtensions.push_back(extension);
}

void TargetGlsl::addRequiredExtension(Stage stage, std::string extension)
{
	m_requiredExtensions[static_cast<std::size_t>(stage)].push_back(std::move(extension));
}

const std::vector<std::string>& TargetGlsl::getRequiredExtensions(Stage stage) const
{
	return m_requiredExtensions[static_cast<std::size_t>(stage)];
}

void TargetGlsl::clearRequiredExtensions()
{
	for (auto& requiredExtensions : m_requiredExtensions)
		requiredExtensions.clear();
}

const std::string& TargetGlsl::getGlslToolCommand(Stage stage) const
{
	return m_glslToolCommand[static_cast<std::size_t>(stage)];
}

void TargetGlsl::setGlslToolCommand(Stage stage, std::string command)
{
	m_glslToolCommand[static_cast<std::size_t>(stage)] = std::move(command);
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
		case Feature::TextureArrays:
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
		case Feature::UniformBlocks:
			if (m_es)
				return m_version >= 300;
			else
				return m_version >= 150;
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
		case Feature::BindingPoints:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 420;
		case Feature::DescriptorSets:
			return false;
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
		// SPIRV-Cross emulates subpass inputs with texelFetch().
		case Feature::SubpassInputs:
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
		case Feature::ClipDistance:
			return !m_es && m_version >= 130;
		case Feature::CullDistance:
			return !m_es && m_version >= 450;
		case Feature::EarlyFragmentTests:
			if (m_es)
				return m_version >= 310;
			else
				return m_version >= 420;
		case Feature::FragmentInputs:
			return false;
	}

	return false;
}

std::vector<std::pair<std::string, std::string>> TargetGlsl::getExtraDefines() const
{
	std::stringstream stream;
	stream << getVersion();
	if (m_es)
		return {{"GLSLES_VERSION", stream.str()}};
	else
		return {{"GLSL_VERSION", stream.str()}};
}

bool TargetGlsl::crossCompile(std::vector<std::uint8_t>& data, Output& output,
	const std::string& fileName, std::size_t line, std::size_t column,
	const std::array<bool, compile::stageCount>&, Stage stage,
	const std::vector<std::uint32_t>& spirv, const std::string&,
	const std::vector<compile::Uniform>&, std::vector<std::uint32_t>&,
	const std::vector<compile::FragmentInputGroup>&, std::uint32_t)
{
	std::size_t stageIndex = static_cast<std::size_t>(stage);

	GlslOutput::Options options;
	options.version = m_version;
	options.es = m_es;
	options.remapDepthRange = m_remapDepthRange;
	options.defaultFloatPrecision = m_defaultFloatPrecision;
	options.defaultIntPrecision = m_defaultIntPrecision;
	options.headerLines = m_headerLines[stageIndex];
	options.requiredExtensions = m_requiredExtensions[stageIndex];
	std::string glsl = GlslOutput::disassemble(output, spirv, options, fileName, line, column);
	data.assign(glsl.begin(), glsl.end());

	// Use external command if set.
	if (!m_glslToolCommand[stageIndex].empty() && !data.empty())
	{
		ExecuteCommand command;
		command.getInput().write(reinterpret_cast<const char*>(data.data()), data.size());
		if (!command.execute(output, m_glslToolCommand[stageIndex]))
			return false;

		data.assign(std::istreambuf_iterator<char>(command.getOutput().rdbuf()),
			std::istreambuf_iterator<char>());
	}

	// Add null terminator so it can be used as a string.
	if (!data.empty())
		data.push_back(0);

	return !data.empty();
}

} // namespace msl
