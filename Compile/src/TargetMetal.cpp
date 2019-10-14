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

#include <MSL/Compile/TargetMetal.h>
#include <MSL/Compile/Output.h>
#include "ExecuteCommand.h"
#include "MetalOutput.h"
#include <fstream>
#include <sstream>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#if MSL_CLANG
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
#endif

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic pop
#endif

namespace msl
{

static void setBinding(std::vector<uint32_t>& spirv, std::uint32_t id, std::uint32_t set,
	std::uint32_t binding)
{
	const unsigned int firstInstruction = 5;
	const std::uint32_t opCodeMask = 0xFFFF;
	const std::uint32_t wordCountShift = 16;
	const std::uint32_t opFunction = 54;
	const std::uint32_t opDecorate = 71;
	const std::uint32_t decorationBinding = 33;
	const std::uint32_t decorationDescriptorSet = 34;

	bool descriptorSet = false, bindingSet = false;
	for (uint32_t i = firstInstruction; i < spirv.size();)
	{
		uint32_t op = spirv[i] & opCodeMask;
		uint32_t wordCount = spirv[i] >> wordCountShift;

		// Once we reach the functions, done with all decorations.
		if (op == opFunction)
			break;

		if (op == opDecorate && spirv[i + 1] == id)
		{
			if (spirv[i + 2] == decorationDescriptorSet)
			{
				spirv[i + 3] = set;
				descriptorSet = true;
			}
			else if (spirv[i + 2] == decorationBinding)
			{
				spirv[i + 3] = binding;
				bindingSet = true;
			}

			if (bindingSet && descriptorSet)
				break;
		}

		i += wordCount;
	}
}

static std::vector<std::uint32_t> setBindingIndices(const std::vector<std::uint32_t>& spirv,
	const std::vector<compile::Uniform>& uniforms, std::vector<std::uint32_t>& uniformIds,
	bool& hasPushConstant, std::uint32_t& bufferCount, std::uint32_t& textureCount)
{
	std::vector<std::uint32_t> adjustedSpirv = spirv;

	hasPushConstant = false;
	for (std::size_t i = 0; i < uniforms.size(); ++i)
	{
		if (uniformIds[i] != unknown && uniforms[i].uniformType == UniformType::PushConstant)
		{
			hasPushConstant = true;
			break;
		}
	}

	std::uint32_t bufferIndex = hasPushConstant;
	std::uint32_t textureIndex = 0;
	for (std::size_t i = 0; i < uniforms.size(); ++i)
	{
		if (uniformIds[i] == unknown)
			continue;

		switch (uniforms[i].uniformType)
		{
			case UniformType::PushConstant:
				uniformIds[i] = 0;
				break;
			case UniformType::Block:
			case UniformType::BlockBuffer:
				setBinding(adjustedSpirv, uniformIds[i], 0, bufferIndex);
				uniformIds[i] = bufferIndex++;
				break;
			case UniformType::Image:
			case UniformType::SampledImage:
			case UniformType::SubpassInput:
				setBinding(adjustedSpirv, uniformIds[i], 1, textureIndex);
				uniformIds[i] = textureIndex++;
				break;
		}
	}

	bufferCount = bufferIndex;
	textureCount = textureIndex;
	return adjustedSpirv;
}

TargetMetal::TargetMetal(std::uint32_t version, Platform platform)
	: m_version(version)
	, m_platform(platform)
{
}

TargetMetal::~TargetMetal()
{
}

TargetMetal::Platform TargetMetal::getPlatform() const
{
	return m_platform;
}

std::uint32_t TargetMetal::getId() const
{
	if (m_platform == Platform::MacOS)
		return MSL_CREATE_ID('M', 'T', 'L', 'X');
	else
		return MSL_CREATE_ID('M', 'T', 'L', 'I');
}

std::uint32_t TargetMetal::getVersion() const
{
	return m_version;
}

bool TargetMetal::featureSupported(Feature feature) const
{
	// TODO: determine supported feature based on version.
	switch (feature)
	{
		case Feature::Std140:
		case Feature::Std430:
		case Feature::BindingPoints:
		case Feature::DescriptorSets:
		case Feature::GeometryStage:
		case Feature::CullDistance:
			return false;
		case Feature::TessellationStages:
			return m_version >= 102;
		case Feature::EarlyFragmentTests:
			return m_version >= 102;
		default:
			return true;
	}
}

std::vector<std::pair<std::string, std::string>> TargetMetal::getExtraDefines() const
{
	std::stringstream stream;
	stream << getVersion();
	std::vector<std::pair<std::string, std::string>> defines = {{"METAL_VERSION", stream.str()}};
	if (m_platform == Platform::MacOS)
		defines.emplace_back("METAL_OSX_VERSION", stream.str());
	else
		defines.emplace_back("METAL_IOS_VERSION", stream.str());
	return defines;
}

void TargetMetal::willCompile()
{
	// Need dummy bindings for internal usage.
	setDummyBindings(true);
}

bool TargetMetal::crossCompile(std::vector<std::uint8_t>& data, Output& output,
	const std::string& fileName, std::size_t line, std::size_t column,
	const std::array<bool, compile::stageCount>& pipelineStages, compile::Stage stage,
	const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
	const std::vector<compile::Uniform>& uniforms, std::vector<std::uint32_t>& uniformIds)
{
	bool outputToBuffer = stage == compile::Stage::Vertex &&
		(pipelineStages[static_cast<int>(compile::Stage::TessellationControl)] ||
			pipelineStages[static_cast<int>(compile::Stage::TessellationEvaluation)]);

	bool hasPushConstant;
	std::uint32_t bufferCount, textureCount;
	std::vector<std::uint32_t> adjustedSpirv = setBindingIndices(spirv, uniforms, uniformIds,
		hasPushConstant, bufferCount, textureCount);

	bool ios = m_platform != Platform::MacOS;
	std::string metal = MetalOutput::disassemble(output, adjustedSpirv, stage, m_version, ios,
		outputToBuffer, hasPushConstant, bufferCount, textureCount, fileName, line, column);
	if (metal.empty())
		return false;

	// Set the entry point back to its original value. The function main0 was set by SPIRV-Cross.
	boost::algorithm::replace_all(metal, "main0", entryPoint);

	// Compile this entry point.
	std::stringstream versionStr;
	if (ios)
		versionStr << "-std=ios-metal";
	else
		versionStr << "-std=osx-metal";
	versionStr << m_version/100 << '.' << m_version % 100;

	std::string extraOptions;
	if (!getStripDebug())
		extraOptions += " -gline-tables-only -MO";

	ExecuteCommand compile(".metal");
	compile.getInput().write(metal.data(), metal.size());
	if (!compile.execute(output, "xcrun -sdk " + getSDK() + " metal -c $input " + versionStr.str() +
		" -o $output" + extraOptions))
	{
		return false;
	}

	ExecuteCommand archive;
	archive.getInput() << compile.getOutput().rdbuf();
	if (!archive.execute(output, "xcrun -sdk " + getSDK() + " metal-ar rcs $output $input"))
		return false;

	ExecuteCommand createLib;
	createLib.getInput() << archive.getOutput().rdbuf();
	if (!createLib.execute(output, "xcrun -sdk " + getSDK() + " metallib $input -o $output"))
		return false;

	data.assign(std::istreambuf_iterator<char>(createLib.getOutput().rdbuf()),
		std::istreambuf_iterator<char>());
	return true;
}

std::string TargetMetal::getSDK() const
{
	switch (m_platform)
	{
		case Platform::MacOS:
			return "macosx";
		case Platform::iOS:
			return "iphoneos";
		case Platform::iOSSimulator:
			return "iphonesimulator";
	}
	return "";
}

} // namespace msl
