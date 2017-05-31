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

TargetMetal::TargetMetal(std::uint32_t version, bool isIos)
	: m_version(version)
	, m_ios(isIos)
	, m_remapDepthRange(false)
	, m_flipVertexY(true)
{
}

TargetMetal::~TargetMetal()
{
}

bool TargetMetal::isIos() const
{
	return m_ios;
}

bool TargetMetal::getRemapDepthRange() const
{
	return m_remapDepthRange;
}

void TargetMetal::setRemapDepthRange(bool remap)
{
	m_remapDepthRange = remap;
}

bool TargetMetal::getFlipVertexY() const
{
	return m_flipVertexY;
}

void TargetMetal::setFlipVertexY(bool flip)
{
	m_flipVertexY = flip;
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
		case Feature::TessellationStages:
		case Feature::GeometryStage:
		case Feature::SubpassInputs:
			return false;
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

bool TargetMetal::crossCompile(std::vector<std::uint8_t>& data, Output& output,
	const std::string& fileName, std::size_t line, std::size_t column, compile::Stage,
	const std::vector<std::uint32_t>& spirv, const std::string& entryPoint)
{

	// Check to see if the entry point was already compiled.
	MetalOutput::Options options;
	options.remapDepthRange = m_remapDepthRange;
	options.flipVertexY = m_flipVertexY;

	std::string metal = MetalOutput::disassemble(output, spirv, options, fileName, line, column);
	if (metal.empty())
		return false;

	// Set the entry point back to its original value. The function mmain was set by SPIRV-Cross.
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
			default:
			{
				std::stringstream stream;
				stream << "invalid version number for OS X Metal: " << getVersion();
				output.addMessage(Output::Level::Error, "", 0, 0, false, stream.str());
				return false;
			}
		}
	}

	ExecuteCommand compile(".metal");
	compile.getInput().write(metal.data(), metal.size());
	if (!compile.execute(output, "xcrun -sdk " + getSDK() + " metal $input " + versionStr +
		" -o $output"))
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
