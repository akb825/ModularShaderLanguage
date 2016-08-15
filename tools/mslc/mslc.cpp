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
#include <MSL/Compile/Output.h>
#include <MSL/Compile/TargetGlsl.h>
#include <MSL/Compile/TargetMetal.h>
#include <MSL/Compile/TargetSpirV.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>

int main(int argc, char** argv)
{
	using namespace boost::program_options;
	options_description mainOptions("main options");
	mainOptions.add_options()
		("help,h", "display this help message")
		("config,c", value<std::string>()->required(), "configuration file describing the target")
		("output,o", value<std::string>()->required(), "output file for the compiled result")
		("include,I", value<std::vector<std::string>>(), "directory to search for includes")
		("define,D", value<std::vector<std::string>>(),
			"add a define for the preprocessor. "
			"A value may optionally be assigned with =. (e.g. -D DEFINE=val)")
		("debug,g", "keep debug symbols")
		("optimize,O", "optimize the compiled result")
		("remap", "remap variable ranges to improve compression of SPIR-V");

	options_description configOptions("options in target configuration file");
	configOptions.add_options()
		("target", value<std::string>()->required(), "the target to compile for. "
			"Possible values are: spirv, glsl, glsl-es, metal-osx, metal-ios")
		("version", value<unsigned int>(), "the version of the target")
		("force-enable", value<std::vector<std::string>>(), "force a feature to be enabled")
		("force-disable", value<std::vector<std::string>>(), "force a feature to be disabled")
		("resources", value<std::string>(), "a path to a file describing custom resource limits. "
			"This uses the same format as glslangValidator.")
		("spirv-command", value<std::string>(), "external command to run on the intermediate "
			"SPIR-V. The string $input will be replaced by the input file path, while the string "
			"$output will be replaced by the output file path.")
		("remap-depth-range", "remap the depth range from [0, 1] to [-1, 1] in the vertex shader "
			"output for GLSL or Metal targets")
		("flip-vertex-y", "flip the vertex y coordinate for Metal targets")
		("flip-fragment-y", "flip the fragment y coordinate for Metal targets")
		("default-float-precision", value<std::string>(), "the default precision to use for "
			"floats in GLSL targets. Possible values are: none, low, medium, high")
		("default-int-precision", value<std::string>(), "the default precision to use for ints in "
			"in GLSL targets. Possible values are: none, low, medium, high")
		("header-line", value<std::vector<std::string>>(), "header line to be added verbatim for "
			"GLSL targets. This will be used for all stages.")
		("header-line-vert", value<std::vector<std::string>>(), "header line to be added "
			"verbatim for GLSL targets. This will be used for the vertex stage.")
		("header-line-tess-ctrl", value<std::vector<std::string>>(), "header line to be added "
			"verbatim for GLSL targets. This will be used for the tessellation control stage.")
		("header-line-tess-eval", value<std::vector<std::string>>(), "header line to be added "
			"verbatim for GLSL targets. This will be used for the tessellation evaluation stage.")
		("header-line-geom", value<std::vector<std::string>>(), "header line to be added verbatim "
			"for GLSL targets. This will be used for the geometry stage.")
		("header-line-frag", value<std::vector<std::string>>(), "header line to be added verbatim "
			"for GLSL targets. This will be used for the fragment stage.")
		("header-line-comp", value<std::vector<std::string>>(), "header line to be added verbatim "
			"for GLSL targets. This will be used for the compute stage.")
		("extension", value<std::vector<std::string>>(), "required extension to be used for GLSL "
			"targets. This will be used for all stages.")
		("extension-vert", value<std::vector<std::string>>(), "required extension to be used for "
			"GLSL targets. This will be used for the vertex stage.")
		("extension-tess-ctrl", value<std::vector<std::string>>(), "required extension to be used "
			"for GLSL targets. This will be used for the tessellation control stage.")
		("extension-tess-eval", value<std::vector<std::string>>(), "required extension to be used "
			"for GLSL targets. This will be used for the tessellation evaluation stage.")
		("extension-geom", value<std::vector<std::string>>(), "required extension to be used "
			"for GLSL targets. This will be used for the geometry stage.")
		("extension-frag", value<std::vector<std::string>>(), "required extension to be used "
			"for GLSL targets. This will be used for the fragment stage.")
		("extension-comp", value<std::vector<std::string>>(), "required extension to be used "
			"for GLSL targets. This will be used for the compute stage.")
		("glsl-command-vert", value<std::string>(), "external command to run on GLSL targets "
			"for the vertex stage. The string $input will be replaced by the input file path, "
			"while the string $output will be replaced by the output file path.")
		("glsl-command-tess-ctrl", value<std::string>(), "external command to run on GLSL targets "
			"for the tessellation control stage. The string $input will be replaced by the input "
			"file path, while the string $output will be replaced by the output file path.")
		("glsl-command-tess-eval", value<std::string>(), "external command to run on GLSL targets "
			"for the tessellation evaluation stage. The string $input will be replaced by the "
			"input file path, while the string $output will be replaced by the output file path.")
		("glsl-command-geom", value<std::string>(), "external command to run on GLSL targets for "
			"the vertex stage. The string $input will be replaced by the input file path, while "
			"the string $output will be replaced by the output file path.")
		("glsl-command-frag", value<std::string>(), "external command to run on GLSL targets for "
			"the fragment stage. The string $input will be replaced by the input file path, while "
			"the string $output will be replaced by the output file path.")
		("glsl-command-comp", value<std::string>(), "external command to run on GLSL targets for "
			"the compute stage. The string $input will be replaced by the input file path, while "
			"the string $output will be replaced by the output file path.");

	positional_options_description positionalOptions;
	positionalOptions.add("input", -1);

	int exitCode = 0;
	variables_map vm;
	try
	{
		store(command_line_parser(argc, argv).
			options(mainOptions).positional(positionalOptions).run(), vm);
		notify(vm);
	}
	catch (std::exception& e)
	{
		if (!vm.count("help"))
		{
			std::cerr << "error: " << e.what() << std::endl << std::endl;
			exitCode = 1;
		}
	}

	bool printHelp = vm.count("help");
	if (!printHelp && exitCode == 0 && !vm.count("input"))
	{
		std::cerr << "error: no input files" << std::endl << std::endl;
		exitCode = 2;
	}

	if (printHelp || exitCode != 0)
	{
		std::cout << "Usage: mslc [options] -c config -o output file1 [file2...]" << std::endl <<
			std::endl;
		std::cout << "Compile one or more shader source files into a shader module." << std::endl <<
			std::endl;
		std::cout << mainOptions << std::endl;

		std::stringstream strstream;
		strstream << configOptions;
		std::string configOptionsStr = strstream.str();
		boost::algorithm::replace_all(configOptionsStr, "--", "  ");
		std::cout << configOptionsStr << std::endl;

		options_description features("features available for force-enable and force-disable");
		for (unsigned int i = 0; i < msl::Target::featureCount; ++i)
		{
			const auto& feature = msl::Target::getFeatureInfo(static_cast<msl::Target::Feature>(i));
			features.add_options()(feature.name, feature.help);
		}

		strstream.str("");
		strstream << features;
		std::string featuresStr = strstream.str();
		boost::algorithm::replace_all(featuresStr, "--", "  ");
		std::cout << featuresStr;
		return exitCode;
	}

	return exitCode;
}
