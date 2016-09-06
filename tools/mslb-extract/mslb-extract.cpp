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

#include <MSL/Client/ModuleCpp.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

using namespace boost::program_options;
using namespace boost::filesystem;

static const char* typeNames[] =
{
	// Scalars and vectors
	"float",
	"vec2",
	"vec3",
	"vec4",
	"double",
	"dvec2",
	"dvec3",
	"dvec4",
	"int",
	"ivec2",
	"ivec3",
	"ivec4",
	"unsigned int",
	"uvec2",
	"uvec3",
	"uvec4",
	"bool",
	"bvec2",
	"bvec3",
	"bvec4",

	// Matrices
	"mat2",
	"mat3",
	"mat4",
	"mat2x3",
	"mat2x4",
	"mat3x2",
	"mat3x4",
	"mat4x2",
	"mat4x3",
	"dmat2",
	"dmat3",
	"dmat4",
	"dmat2x3",
	"dmat2x4",
	"dmat3x2",
	"dmat3x4",
	"dmat4x2",
	"dmat4x3",

	// Samplers
	"sampler1D",
	"sampler2D",
	"sampler3D",
	"samplerCube",
	"sampler1DShadow",
	"sampler2DShadow",
	"sampler1DArray",
	"sampler2DArray",
	"sampler1DArrayShadow",
	"sampler2DArrayShadow",
	"sampler2DMS",
	"sampler2DMSArray",
	"samplerCubeShadow",
	"samplerBuffer",
	"sampler2DRect",
	"sampler2DRectShadow",
	"isampler1D",
	"isampler2D",
	"isampler3D",
	"isamplerCube",
	"isampler1DArray",
	"isampler2DArray",
	"isampler2DMS",
	"isampler2DMSArray",
	"isampler2DRect",
	"usampler1D",
	"usampler2D",
	"usampler3D",
	"usamplerCube",
	"usampler1DArray",
	"usampler2DArray",
	"usampler2DMS",
	"usampler2DMSArray",
	"usampler2DRect",

	// Images
	"image1D",
	"image2D",
	"image3D",
	"imageCube",
	"image1DArray",
	"image2DArray",
	"image2DMS",
	"image2DMSArray",
	"imageBuffer",
	"image2DRect",
	"iimage1D",
	"iimage2D",
	"iimage3D",
	"iimageCube",
	"iimage1DArray",
	"iimage2DArray",
	"iimage2DMS",
	"iimage2DMSArray",
	"iimage2DRect",
	"uimage1D",
	"uimage2D",
	"uimage3D",
	"uimageCube",
	"uimage1DArray",
	"uimage2DArray",
	"uimage2DMS",
	"uimage2DMSArray",
	"uimage2DRect",

	// Other
	"subpassInput",
	"subpassInputMS",
	"isubpassInput",
	"isubpassInputMS",
	"usubpassInput",
	"usubpassInputMS",
};
static_assert(sizeof(typeNames)/sizeof(*typeNames) == msl::Module::typeCount,
	"typeNames out of sync with enum");

static const char* stageExtensions[] =
{
	".vert",
	".tesc",
	".tese",
	".geom",
	".frag",
	".comp"
};
static_assert(sizeof(stageExtensions)/sizeof(*stageExtensions) == msl::Module::stageCount,
	"stageExtensions out of sync with enum");

static const char* stageNames[] =
{
	"vertex",
	"tessellation-control",
	"tessellation-evaluation",
	"geometry",
	"fragment",
	"compute"
};
static_assert(sizeof(stageNames)/sizeof(*stageNames) == msl::Module::stageCount,
	"stageNames out of sync with enum");

static bool shadersAreText(const msl::Module& module)
{
	uint32_t targetId = module.targetId();
	return
		targetId == MSL_CREATE_ID('G', 'L', 'S', 'L') ||
		targetId == MSL_CREATE_ID('G', 'L', 'E', 'S') ||
		targetId == MSL_CREATE_ID('M', 'T', 'L', 'X') ||
		targetId == MSL_CREATE_ID('M', 'T', 'L', 'I');
}

static bool openFile(std::ofstream& stream, const std::string& fileName, bool binary)
{
	std::ios_base::openmode options = std::ios_base::trunc;
	if (binary)
		options |= std::ios_base::binary;
	stream.open(fileName, options);
	if (!stream.is_open())
	{
		std::cerr << "error: could not open file: " << fileName <<std::endl;
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	// Specify the options.
	options_description mainOptions("options");
	mainOptions.add_options()
		("help,h", "display this help message")
		("input,i", value<std::string>()->required(),
			"input shader module file to extract")
		("output,o", value<std::string>()->required(), "output directory to extract to. This will "
			"be created if it doesn't exist.");

	positional_options_description positionalOptions;
	positionalOptions.add("input", 1);

	int exitCode = 0;
	variables_map options;
	try
	{
		store(command_line_parser(argc, argv).
			options(mainOptions).positional(positionalOptions).run(), options);
		notify(options);
	}
	catch (std::exception& e)
	{
		if (!options.count("help"))
		{
			std::cerr << "error: " << e.what() << std::endl << std::endl;
			exitCode = 1;
		}
	}

	if (options.count("help") || exitCode != 0)
	{
		std::cout << "Usage: mslb-extract -o output file" << std::endl << std::endl;
		std::cout << "Extract a compiled shader module into its components." << std::endl <<
			std::endl;
		std::cout <<
			"The shader for each pipeline stage will be written to the output directory. The\n"
			"name will be based on the module name, pipelien name, and have an extension \n"
			"based on the stage.\n"
			"    <module>.<pipeline>.vert\n"
			"    <module>.<pipeline>.tesc\n"
			"    <module>.<pipeline>.tese\n"
			"    <module>.<pipeline>.frag\n"
			"    <module>.<pipeline>.geom\n"
			"    <module>.<pipeline>.comp\n\n"
			"Unused stages will have no output file. The format of the file will depend on \n"
			"the target, and may either be text or binary.\n\n"
			"Additionally, the following two files will be output:\n"
			"    <module>.json: json file describing each pipeline in the module.\n"
			"    <module>.shared: the shared data (only for certain targets)" <<
			std::endl << std::endl;

		std::cout << mainOptions;
		return exitCode;
	}

	msl::Module module;
	std::string moduleFile = options["input"].as<std::string>();
	if (!module.read(moduleFile))
	{
		std::cerr << "error: could not read shader module: " << moduleFile << std::endl;
		return 2;
	}

	path outputDir = options["output"].as<std::string>();
	boost::system::error_code ec;
	create_directories(outputDir, ec);
	if (ec)
	{
		std::cerr << "error: could not create directory: " << outputDir.string() << std::endl;
		return 3;
	}

	// Write out json description.
	std::string moduleName = path(moduleFile).filename().replace_extension().string();
	std::string jsonFileName = (outputDir/(moduleName + ".json")).string();
	std::ofstream jsonFile;
	if (!openFile(jsonFile, jsonFileName, false))
		return 3;

	uint32_t targetId = module.targetId();
	jsonFile << "{\n";

	// Target info
	jsonFile << "\t\"targetId\": \"" <<
		static_cast<char>(targetId >> 24) <<
		static_cast<char>(targetId >> 16) <<
		static_cast<char>(targetId >> 8) <<
		static_cast<char>(targetId) << "\",\n";
	jsonFile << "\t\"targetVersion\": " << module.targetVersion() << ",\n";

	// Pipelines
	jsonFile << "\t\"pipelines\": \n\t[\n";
	bool textShaders = shadersAreText(module);
	uint32_t pipelineCount = module.pipelineCount();
	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		jsonFile << "\t\t{\n";

		const char* pipelineName = module.pipelineName(i);
		jsonFile << "\t\t\t\"name\": \"" << pipelineName << "\",\n";

		// Stage shaders
		for (unsigned int j = 0; j < msl::Module::stageCount; ++j)
		{
			auto stage = static_cast<msl::Module::Stage>(j);
			uint32_t shader = module.pipelineShader(i, stage);
			if (shader == msl::Module::unknown)
				continue;

			std::string shaderName = moduleName + "." + pipelineName + stageExtensions[j];
			jsonFile << "\t\t\t\"" << stageNames[j] << "\": \"" << shaderName << "\",\n";

			std::string shaderFileName = (outputDir/shaderName).string();
			std::ofstream shaderStream;
			if (!openFile(shaderStream, shaderFileName, !textShaders))
				return 3;

			uint32_t writeSize = module.shaderSize(shader);
			// Remove null terminator for text shaders.
			if (writeSize > 0 && textShaders)
				--writeSize;
			shaderStream.write(reinterpret_cast<const char*>(module.shaderData(shader)), writeSize);
		}

		// Unfiroms
		jsonFile << "\t\t\t\"uniforms\": \n\t\t\t[\n";
		uint32_t uniformCount = module.uniformCount(i);
		for (uint32_t j = 0; j < uniformCount; ++j)
		{
			jsonFile << "\t\t\t\t{\n";

			jsonFile << "\t\t\t\t\t\"name\": \"" << module.uniformName(i, j) << "\",\n";
			unsigned int type = static_cast<unsigned int>(module.uniformType(i, j));
			if (type < msl::Module::typeCount)
				jsonFile << "\t\t\t\t\t\"type\": \"" << typeNames[type] << "\",\n";
			else
				jsonFile << "\t\t\t\t\t\"type\": \"invalid\",\n";
			jsonFile << "\t\t\t\t\t\"blockIndex\": " <<
				static_cast<int>(module.uniformBlockIndex(i, j)) << ",\n";
			jsonFile << "\t\t\t\t\t\"blockOffset\": " <<
				static_cast<int>(module.uniformBufferOffset(i, j)) << ",\n";
			jsonFile << "\t\t\t\t\t\"elements\": " << module.uniformElements(i, j) << "\n";

			if (j == uniformCount - 1)
				jsonFile << "\t\t\t\t}\n";
			else
				jsonFile << "\t\t\t\t},\n";
		}
		jsonFile << "\t\t\t],\n";

		// Unfirom blocks
		jsonFile << "\t\t\t\"uniformBlocks\": \n\t\t\t[\n";
		uint32_t uniformBlockCount = module.uniformBlockCount(i);
		for (uint32_t j = 0; j < uniformBlockCount; ++j)
		{
			jsonFile << "\t\t\t\t{\n";

			jsonFile << "\t\t\t\t\t\"name\": \"" << module.uniformBlockName(i, j) << "\",\n";
			jsonFile << "\t\t\t\t\t\"size\": " << module.uniformBlockSize(i, j) << "\n";

			if (j == uniformBlockCount - 1)
				jsonFile << "\t\t\t\t}\n";
			else
				jsonFile << "\t\t\t\t},\n";
		}
		jsonFile << "\t\t\t],\n";

		// Attributes
		jsonFile << "\t\t\t\"attributes\": \n\t\t\t[\n";
		uint32_t attributesCount = module.attributeCount(i);
		for (uint32_t j = 0; j < attributesCount; ++j)
		{
			jsonFile << "\t\t\t\t{\n";

			jsonFile << "\t\t\t\t\t\"name\": \"" << module.attributeName(i, j) << "\",\n";
			unsigned int type = static_cast<unsigned int>(module.attributeType(i, j));
			if (type < msl::Module::typeCount)
				jsonFile << "\t\t\t\t\t\"type\": \"" << typeNames[type] << "\"\n";
			else
				jsonFile << "\t\t\t\t\t\"type\": \"invalid\"\n";

			if (j == attributesCount - 1)
				jsonFile << "\t\t\t\t}\n";
			else
				jsonFile << "\t\t\t\t},\n";
		}
		jsonFile << "\t\t\t]\n";

		if (i == pipelineCount - 1)
			jsonFile << "\t\t}\n";
		else
			jsonFile << "\t\t},\n";
	}


	uint32_t sharedDataSize = module.sharedDataSize();
	if (sharedDataSize > 0)
	{
		jsonFile << "\t],\n";
		std::string sharedName = moduleName + ".shared";
		jsonFile << "\t\"sharedData\": \"" << sharedName << "\"\n}\n";

		std::string sharedFileName = (outputDir/sharedName).string();
		std::ofstream sharedStream;
		if (!openFile(sharedStream, sharedFileName, false))
			return 3;

		sharedStream.write(reinterpret_cast<const char*>(module.sharedData()), sharedDataSize);
	}
	else
		jsonFile << "\t]\n}\n";

	std::cout << "extracted module contents to " << outputDir.string() << std::endl;
	return 0;
}
