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
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

namespace msl
{

static void setBinding(std::vector<uint32_t>& spirv, std::uint32_t id, std::uint32_t index)
{
	const unsigned int firstInstruction = 5;
	const std::uint32_t opCodeMask = 0xFFFF;
	const std::uint32_t wordCountShift = 16;
	const std::uint32_t opFunction = 54;
	const std::uint32_t opDecorate = 71;
	const std::uint32_t decorationBinding = 33;

	for (uint32_t i = firstInstruction; i < spirv.size();)
	{
		uint32_t op = spirv[i] & opCodeMask;
		uint32_t wordCount = spirv[i] >> wordCountShift;

		// Once we reach the functions, done with all decorations.
		if (op == opFunction)
			break;

		if (op == opDecorate && spirv[i + 1] == id && spirv[i + 2] == decorationBinding)
		{
			spirv[i + 3] = index;
			break;
		}

		i += wordCount;
	}
}

static std::vector<std::uint32_t> setBindingIndices(const std::vector<std::uint32_t>& spirv,
	const std::vector<compile::Uniform>& uniforms, std::vector<std::uint32_t>& uniformIds)
{
	std::vector<std::uint32_t> adjustedSpirv = spirv;

	bool hasPushConstant = false;
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
				setBinding(adjustedSpirv, uniformIds[i], bufferIndex);
				uniformIds[i] = bufferIndex++;
				break;
			case UniformType::Image:
			case UniformType::SampledImage:
			case UniformType::SubpassInput:
				setBinding(adjustedSpirv, uniformIds[i], textureIndex);
				uniformIds[i] = textureIndex++;
				break;
		}
	}

	return adjustedSpirv;
}

TargetMetal::TargetMetal(std::uint32_t version, bool isIos)
	: m_version(version)
	, m_ios(isIos)
{
}

TargetMetal::~TargetMetal()
{
}

bool TargetMetal::isIos() const
{
	return m_ios;
}

std::uint32_t TargetMetal::getId() const
{
	if (m_ios)
		return MSL_CREATE_ID('M', 'T', 'L', 'I');
	else
		return MSL_CREATE_ID('M', 'T', 'L', 'X');
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
			return false;
		case Feature::TessellationStages:
			return m_version >= 12;
		default:
			return true;
	}
}

std::vector<std::pair<std::string, std::string>> TargetMetal::getExtraDefines() const
{
	std::stringstream stream;
	stream << getVersion();
	if (m_ios)
		return {{"METAL_IOS_VERSION", stream.str()}};
	else
		return {{"METAL_OSX_VERSION", stream.str()}};
}

void TargetMetal::willCompile()
{
	// Need dummy bindings for internal usage.
	setDummyBindings(true);
}

bool TargetMetal::crossCompile(std::vector<std::uint8_t>& data, Output& output,
	const std::string& fileName, std::size_t line, std::size_t column, compile::Stage,
	const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
	const std::vector<compile::Uniform>& uniforms, std::vector<std::uint32_t>& uniformIds)
{
	std::vector<std::uint32_t> adjustedSpirv = setBindingIndices(spirv, uniforms, uniformIds);
	std::string metal = MetalOutput::disassemble(output, adjustedSpirv, m_version, m_ios, fileName,
		line, column);
	if (metal.empty())
		return false;

	// Set the entry point back to its original value. The function main0 was set by SPIRV-Cross.
	boost::algorithm::replace_all(metal, "main0", entryPoint);

	// Compile this entry point.
	std::string versionStr;
	if (m_ios)
	{
		switch (getVersion())
		{
			case 10:
				versionStr = "-std=ios-metal1.0";
				break;
			case 11:
				versionStr = "-std=ios-metal1.1";
				break;
			case 12:
				versionStr = "-std=ios-metal1.2";
				break;
			case 20:
				versionStr = "-std=ios-metal2.0";
				break;
			case 21:
				versionStr = "-std=ios-metal2.1";
				break;
			default:
			{
				std::stringstream stream;
				stream << "invalid version number for iOS Metal: " << getVersion();
				output.addMessage(Output::Level::Error, "", 0, 0, false, stream.str());
				return false;
			}
		}
	}
	else
	{
		switch (getVersion())
		{
			case 11:
				versionStr = "-std=osx-metal1.1";
				break;
			case 12:
				versionStr = "-std=osx-metal1.2";
				break;
			case 20:
				versionStr = "-std=osx-metal2.0";
				break;
			case 21:
				versionStr = "-std=osx-metal2.1";
			default:
			{
				std::stringstream stream;
				stream << "invalid version number for OS X Metal: " << getVersion();
				output.addMessage(Output::Level::Error, "", 0, 0, false, stream.str());
				return false;
			}
		}
	}

	std::string extraOptions;
	if (!getStripDebug())
		extraOptions += " -gline-tables-only -MO";

	ExecuteCommand compile(".metal");
	compile.getInput().write(metal.data(), metal.size());
	if (!compile.execute(output, "xcrun -sdk " + getSDK() + " metal -c $input " + versionStr +
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
	if (m_ios)
		return "iphoneos";
	else
		return "macosx";
}

} // namespace msl
