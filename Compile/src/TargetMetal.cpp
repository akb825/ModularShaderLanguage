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
	, m_remapDepthRange(true)
	, m_flipVertexY(true)
	, m_flipFragmentY(true)
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

bool TargetMetal::getFlipFragmentY() const
{
	return m_flipFragmentY;
}

void TargetMetal::setFlipFragmentY(bool flip)
{
	m_flipFragmentY = flip;
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
		case Feature::TessellationStages:
		case Feature::GeometryStage:
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
	const std::vector<std::uint32_t>& spirv, const std::string& entryPoint,
	const std::string& fileName, std::size_t line, std::size_t column)
{

	// Check to see if the entry point was already compiled.
	MetalOutput::Options options;
	options.remapDepthRange = m_remapDepthRange;
	options.flipVertexY = m_flipVertexY;
	options.flipFragmentY = m_flipFragmentY;

	std::string metal = MetalOutput::disassemble(output, spirv, options, fileName, line, column);
	if (metal.empty())
		return false;

	// Set the entry point back to its original value. The function mmain was set by SPIRV-Cross.
	boost::algorithm::replace_all(metal, "mmain", entryPoint);

	// Check if the entry point was already added. Make sure the generated code was the same if so.
	auto foundIter = m_entryPointData.find(entryPoint);
	if (foundIter != m_entryPointData.end())
	{
		if (foundIter->second.metal != metal)
		{
			output.addMessage(Output::Level::Error, fileName, line, column, false,
				"entry point gave a different compiled result: " + entryPoint);
			output.addMessage(Output::Level::Error, foundIter->second.fileName,
				foundIter->second.line, foundIter->second.column, true,
				"see pipeline for previously compiled entry point");
			return false;
		}

		data.assign(entryPoint.begin(), entryPoint.end());
		return true;
	}

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
				versionStr = "-std=ios-metal1.1";
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

	ExecuteCommand compile;
	compile.getInput().write(metal.data(), metal.size());
	if (!compile.execute(output, "metal $input -W " + versionStr + " -o $output"))
		return false;

	std::vector<std::uint8_t> compiledData(
		std::istreambuf_iterator<char>(compile.getOutput().rdbuf()),
		std::istreambuf_iterator<char>());

	// Add the compiled data for the input.
	CompiledDataInfo info;
	info.metal = std::move(metal);
	info.data = std::move(compiledData);
	info.fileName = fileName;
	info.line = line;
	info.column = column;
	m_entryPointData.emplace(entryPoint, std::move(info));

	data.assign(entryPoint.begin(), entryPoint.end());
	return true;
}

bool TargetMetal::getSharedData(std::vector<std::uint8_t>& data, Output& output)
{
	if (m_entryPointData.empty())
		return true;

	std::string archiveCommand = "metal-ar -r $output";
	std::vector<std::string> entryPointFiles;
	for (const auto& entryPointData : m_entryPointData)
	{
		std::string fileName =
			(boost::filesystem::temp_directory_path()/boost::filesystem::unique_path()).string();
		std::fstream stream(fileName);
		stream.write(reinterpret_cast<const char*>(entryPointData.second.data.data()),
			entryPointData.second.data.size());
		archiveCommand.push_back(' ');
		archiveCommand += fileName;
		entryPointFiles.push_back(std::move(fileName));
	}

	ExecuteCommand archive, createlib;
	if (!archive.execute(output, archiveCommand))
		return false;

	createlib.getInput() << archive.getOutput().rdbuf();
	if (!createlib.execute(output, "metallib $input -o $output"))
		return false;

	data.assign(std::istreambuf_iterator<char>(createlib.getOutput().rdbuf()),
		std::istreambuf_iterator<char>());
	return true;
}

} // namespace msl
