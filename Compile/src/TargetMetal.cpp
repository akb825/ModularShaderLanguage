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

#include <MSL/Compile/TargetMetal.h>
#include <MSL/Compile/Output.h>
#include "ExecuteCommand.h"
#include "MetalOutput.h"
#include <cassert>
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

using namespace compile;

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
	const std::vector<Uniform>& uniforms, std::vector<std::uint32_t>& uniformIds,
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

static std::string setFragmentGroup(const std::string& metal, const std::string& entryPoint,
	std::uint32_t fragmentGroup)
{
	std::string structDecl = "struct " + entryPoint + "_out";
	std::size_t outStructStart = metal.find(structDecl);
	if (outStructStart == std::string::npos)
		return metal;

	std::size_t outStructEnd = metal.find('}', outStructStart);
	assert(outStructEnd != std::string::npos);

	// Add raster order group to all color elements.
	std::string rasterOrderGroup = ", raster_order_group(" + std::to_string(fragmentGroup) + ")";
	std::string result = metal.substr(0, outStructStart);
	std::size_t curPos = outStructStart;
	do
	{
		std::size_t colorStart = metal.find("color(", curPos);
		if (colorStart > outStructEnd)
			break;

		std::size_t colorEnd = metal.find(')', colorStart);
		assert(colorEnd < outStructEnd);

		result += metal.substr(curPos, colorEnd - curPos + 1);
		result += rasterOrderGroup;
		curPos = colorEnd + 1;
	} while (true);

	result += metal.substr(curPos);
	return result;
}

static std::string patchEntryPointInputGroup(const std::string& metal,
	const FragmentInputGroup& inputGroup)
{
	// Expect one primary declaration in the entry point.
	std::string declStart = "constant " + inputGroup.type + "& " + inputGroup.name + " [[buffer(";
	std::size_t startIdx = metal.find(declStart);
	if (startIdx == std::string::npos)
		return metal;

	std::size_t endIdx = metal.find("]]", startIdx);
	assert(endIdx != std::string::npos);
	endIdx += 2;
	std::string result = metal.substr(0, startIdx);
	result += inputGroup.type;
	result += ' ';
	result += inputGroup.name;
	result += metal.substr(endIdx);
	return result;
}

static std::string patchFragmentInputs(const std::string& metal,
	const FragmentInputGroup& inputGroup)
{
	std::string fixedParamResult = patchEntryPointInputGroup(metal, inputGroup);
	// Need to also replace "constant <type>" with "thread <type>" for other parameters.
	boost::algorithm::replace_all(fixedParamResult, "constant " + inputGroup.type,
		"thread " + inputGroup.type);

	// Add [[color(location), raster_order_group(fragmentGroup)]] to each member.
	std::size_t structStart = fixedParamResult.find("struct " + inputGroup.type);
	if (structStart == std::string::npos)
		return fixedParamResult;

	std::size_t structEnd = fixedParamResult.find('}', structStart);
	assert(structEnd != std::string::npos);

	std::string structText = fixedParamResult.substr(structStart, structEnd - structStart);
	for (const FragmentInput& input : inputGroup.inputs)
	{
		std::string replaceString = input.name + " [[color(" + std::to_string(input.location) +
			"), raster_order_group(" + std::to_string(input.fragmentGroup) + ")]]";
		boost::algorithm::replace_first(structText, input.name, replaceString);
	}

	std::string result = fixedParamResult.substr(0, structStart);
	result += structText;
	result += fixedParamResult.substr(structEnd);
	return result;
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
		case Feature::FragmentInputs:
			return (m_platform == Platform::iOS && m_version >= 200) || m_version >= 203;
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

bool TargetMetal::compileMetal(std::vector<std::uint8_t>& data, Output& output,
	const std::string& metal)
{
	// Compile this entry point.
	std::stringstream versionStr;
	if (m_platform != Platform::MacOS)
		versionStr << "-std=ios-metal";
	else
		versionStr << "-std=macos-metal";
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

bool TargetMetal::crossCompile(std::vector<std::uint8_t>& data, Output& output,
	const std::string& fileName, std::size_t line, std::size_t column,
	const std::array<bool, compile::stageCount>& pipelineStages, compile::Stage stage,
	const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
	const std::vector<compile::Uniform>& uniforms, std::vector<std::uint32_t>& uniformIds,
	const std::vector<compile::FragmentInputGroup>& fragmentInputs,
	std::uint32_t fragmentGroup)
{
	bool outputToBuffer = stage == Stage::Vertex &&
		(pipelineStages[static_cast<int>(Stage::TessellationControl)] ||
			pipelineStages[static_cast<int>(Stage::TessellationEvaluation)]);

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

	// Need to patch the generated Metal source code when using frament inputs.
	if (stage == Stage::Fragment && featureEnabled(Feature::FragmentInputs))
	{
		if (fragmentGroup != unknown)
			metal = setFragmentGroup(metal, entryPoint, fragmentGroup);

		for (const FragmentInputGroup& inputGroup : fragmentInputs)
			metal = patchFragmentInputs(metal, inputGroup);
	}

	return compileMetal(data, output, metal);
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
