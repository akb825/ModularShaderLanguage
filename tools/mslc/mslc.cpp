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
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>

using namespace boost::program_options;

static std::string filterHeader(const std::string& line)
{
	std::string filtered = line;
	std::replace(filtered.begin(), filtered.end(), '@', '#');
	return filtered;
}

static std::unique_ptr<msl::Target> createGlslTarget(const std::string& targetName,
	const variables_map& config, const std::string& configFilePath)
{
	bool es = targetName == "glsl-es";
	unsigned int version;
	if (config.count("version"))
	{
		std::string versionStr = config["version"].as<std::string>();
		try
		{
			version = boost::lexical_cast<unsigned int>(versionStr);
		}
		catch (...)
		{
			std::cerr << configFilePath << " error: invalid version: " << versionStr <<
				std::endl << std::endl;
			return nullptr;
		}
	}
	else
	{
		std::cerr << configFilePath << " error: version not provided" << std::endl <<
			std::endl;
		return nullptr;
	}

	std::unique_ptr<msl::TargetGlsl> target(new msl::TargetGlsl(version, es));

	if (config.count("remap-depth-range"))
		target->setRemapDepthRange(config["remap-depth-range"].as<bool>());

	if (config.count("default-float-precision"))
	{
		std::string precision = config["default-float-precision"].as<std::string>();
		if (precision == "none")
			target->setDefaultFloatPrecision(msl::TargetGlsl::Precision::None);
		else if (precision == "low")
			target->setDefaultFloatPrecision(msl::TargetGlsl::Precision::Low);
		else if (precision == "medium")
			target->setDefaultFloatPrecision(msl::TargetGlsl::Precision::Medium);
		else if (precision == "high")
			target->setDefaultFloatPrecision(msl::TargetGlsl::Precision::High);
		else
		{
			std::cerr << configFilePath << " error: unknown precision: " << precision <<
				std::endl << std::endl;
			return nullptr;
		}
	}

	if (config.count("default-int-precision"))
	{
		std::string precision = config["default-int-precision"].as<std::string>();
		if (precision == "none")
			target->setDefaultIntPrecision(msl::TargetGlsl::Precision::None);
		else if (precision == "low")
			target->setDefaultIntPrecision(msl::TargetGlsl::Precision::Low);
		else if (precision == "medium")
			target->setDefaultIntPrecision(msl::TargetGlsl::Precision::Medium);
		else if (precision == "high")
			target->setDefaultIntPrecision(msl::TargetGlsl::Precision::High);
		else
		{
			std::cerr << configFilePath << " error: unknown precision: " << precision <<
				std::endl << std::endl;
			return nullptr;
		}
	}

	if (config.count("header-line"))
	{
		for (const std::string& str : config["header-line"].as<std::vector<std::string>>())
			target->addHeaderLine(filterHeader(str));
	}

	if (config.count("header-line-vert"))
	{
		for (const std::string& str : config["header-line-vert"].as<std::vector<std::string>>())
			target->addHeaderLine(msl::compile::Stage::Vertex, filterHeader(str));
	}

	if (config.count("header-line-tess-ctrl"))
	{
		for (const std::string& str : config["header-line-tess-ctrl"].as<std::vector<std::string>>())
			target->addHeaderLine(msl::compile::Stage::TessellationControl, filterHeader(str));
	}

	if (config.count("header-line-tess-eval"))
	{
		for (const std::string& str : config["header-line-tess-eval"].as<std::vector<std::string>>())
			target->addHeaderLine(msl::compile::Stage::TessellationEvaluation, filterHeader(str));
	}

	if (config.count("header-line-geom"))
	{
		for (const std::string& str : config["header-line-geom"].as<std::vector<std::string>>())
			target->addHeaderLine(msl::compile::Stage::Geometry, filterHeader(str));
	}

	if (config.count("header-line-frag"))
	{
		for (const std::string& str : config["header-line-frag"].as<std::vector<std::string>>())
			target->addHeaderLine(msl::compile::Stage::Fragment, filterHeader(str));
	}

	if (config.count("header-line-comp"))
	{
		for (const std::string& str : config["header-line-comp"].as<std::vector<std::string>>())
			target->addHeaderLine(msl::compile::Stage::Compute, filterHeader(str));
	}

	if (config.count("extension"))
	{
		for (const std::string& str : config["extension"].as<std::vector<std::string>>())
			target->addRequiredExtension(str);
	}

	if (config.count("extension-vert"))
	{
		for (const std::string& str : config["extension-vert"].as<std::vector<std::string>>())
			target->addRequiredExtension(msl::compile::Stage::Vertex, str);
	}

	if (config.count("extension-tess-ctrl"))
	{
		for (const std::string& str : config["extension-tess-ctrl"].as<std::vector<std::string>>())
			target->addRequiredExtension(msl::compile::Stage::TessellationControl, str);
	}

	if (config.count("extension-tess-eval"))
	{
		for (const std::string& str : config["extension-tess-eval"].as<std::vector<std::string>>())
			target->addRequiredExtension(msl::compile::Stage::TessellationEvaluation, str);
	}

	if (config.count("extension-geom"))
	{
		for (const std::string& str : config["extension-geom"].as<std::vector<std::string>>())
			target->addRequiredExtension(msl::compile::Stage::Geometry, str);
	}

	if (config.count("extension-frag"))
	{
		for (const std::string& str : config["extension-frag"].as<std::vector<std::string>>())
			target->addRequiredExtension(msl::compile::Stage::Fragment, str);
	}

	if (config.count("extension-comp"))
	{
		for (const std::string& str : config["extension-comp"].as<std::vector<std::string>>())
			target->addRequiredExtension(msl::compile::Stage::Compute, str);
	}

	if (config.count("glsl-command-vert"))
	{
		target->setGlslToolCommand(msl::compile::Stage::Vertex,
			config["glsl-command-vert"].as<std::string>());
	}

	if (config.count("glsl-command-tess-ctrl"))
	{
		target->setGlslToolCommand(msl::compile::Stage::TessellationControl,
			config["glsl-command-tess-ctrl"].as<std::string>());
	}

	if (config.count("glsl-command-tess-eval"))
	{
		target->setGlslToolCommand(msl::compile::Stage::TessellationEvaluation,
			config["glsl-command-tess-eval"].as<std::string>());
	}

	if (config.count("glsl-command-geom"))
	{
		target->setGlslToolCommand(msl::compile::Stage::Geometry,
			config["glsl-command-geom"].as<std::string>());
	}

	if (config.count("glsl-command-frag"))
	{
		target->setGlslToolCommand(msl::compile::Stage::Fragment,
			config["glsl-command-frag"].as<std::string>());
	}

	if (config.count("glsl-command-comp"))
	{
		target->setGlslToolCommand(msl::compile::Stage::Compute,
			config["glsl-command-comp"].as<std::string>());
	}

	return std::move(target);
}

static std::unique_ptr<msl::Target> createMetalTarget(const std::string& targetName,
	const variables_map& config, const std::string& configFilePath)
{
	bool ios = targetName == "metal-ios";
	unsigned int version;
	if (config.count("version"))
	{
		std::string versionStr = config["version"].as<std::string>();
		if (versionStr == "1.0")
			version = 10;
		else if (versionStr == "1.1")
			version = 11;
		else
		{
			std::cerr << configFilePath << " error: invalid version: " << versionStr <<
				std::endl << std::endl;
			return nullptr;
		}
	}
	else
	{
		std::cerr << configFilePath << " error: version not provided" << std::endl <<
			std::endl;
		return nullptr;
	}

	std::unique_ptr<msl::TargetMetal> target(new msl::TargetMetal(version, ios));

	return std::move(target);
}

static std::pair<std::string, std::string> splitDefineString(const std::string& str)
{
	std::size_t equalIndex = str.find_first_of('=');
	std::pair<std::string, std::string> definePair;
	if (equalIndex == std::string::npos)
		definePair.first = str;
	else
	{
		definePair.first = str.substr(0, equalIndex);
		definePair.second = str.substr(equalIndex + 1);
	}

	boost::algorithm::trim(definePair.first);
	boost::algorithm::trim(definePair.second);
	return definePair;
}

static bool setCommonTargetConfig(msl::Target& target, const variables_map& options,
	const variables_map& config, const std::string& configFilePath)
{
	std::unordered_map<std::string, msl::Target::Feature> featureMap;
	for (unsigned int i = 0; i < msl::Target::featureCount; ++i)
	{
		auto feature = static_cast<msl::Target::Feature>(i);
		const auto& info = msl::Target::getFeatureInfo(feature);
		featureMap.emplace(info.name, feature);
	}

	if (config.count("force-enable"))
	{
		for (const std::string& str : config["force-enable"].as<std::vector<std::string>>())
		{
			auto foundIter = featureMap.find(str);
			if (foundIter == featureMap.end())
			{
				std::cerr << configFilePath << " error: unknown feature: " << str << std::endl <<
					std::endl;
				return false;
			}

			target.overrideFeature(foundIter->second, true);
		}
	}

	if (config.count("force-disable"))
	{
		for (const std::string& str : config["force-disable"].as<std::vector<std::string>>())
		{
			auto foundIter = featureMap.find(str);
			if (foundIter == featureMap.end())
			{
				std::cerr << configFilePath << " error: unknown feature: " << str << std::endl <<
					std::endl;
				return false;
			}

			target.overrideFeature(foundIter->second, false);
		}
	}

	if (config.count("resources"))
		target.setResourcesFileName(config["resources"].as<std::string>());

	if (config.count("spirv-command"))
		target.setSpirVToolCommand(config["spirv-command"].as<std::string>());

	// Add inlcudes and defines.
	if (options.count("include"))
	{
		for (const std::string& str : options["include"].as<std::vector<std::string>>())
			target.addIncludePath(str);
	}

	// Commind-line option defines
	if (options.count("define"))
	{
		for (const std::string& str : options["define"].as<std::vector<std::string>>())
		{
			auto definePair = splitDefineString(str);
			target.addDefine(std::move(definePair.first), std::move(definePair.second));
		}
	}

	// Config file defines
	if (config.count("define"))
	{
		for (const std::string& str : config["define"].as<std::vector<std::string>>())
		{
			auto definePair = splitDefineString(str);
			target.addDefine(std::move(definePair.first), std::move(definePair.second));
		}
	}

	if (config.count("pre-header-line"))
	{
		for (const std::string& str : config["pre-header-line"].as<std::vector<std::string>>())
			target.addPreHeaderLine(filterHeader(str));
	}

	if (config.count("remap-variables"))
		target.setRemapVariables(config["remap-variables"].as<bool>());

	if (config.count("dummy-bindings"))
		target.setDummyBindings(config["dummy-bindings"].as<bool>());

	if (config.count("adjustable-bindings"))
		target.setAdjustableBindings(config["adjustable-bindings"].as<bool>());

	target.setStripDebug(options.count("strip") > 0);
	target.setOptimize(options.count("optimize") > 0);

	return true;
}

static void printOutput(msl::Output& output, bool printWarnings)
{
	const char* continueStr = "note: ";
	for (const msl::Output::Message& message : output.getMessages())
	{
		std::ostream* stream;
		const char* levelStr;
		switch (message.level)
		{
			case msl::Output::Level::Error:
				levelStr = "error: ";
				stream = &std::cerr;
				break;
			case msl::Output::Level::Warning:
				if (!printWarnings)
					continue;
				levelStr = "warning: ";
				stream = &std::cerr;
				break;
			case msl::Output::Level::Info:
				levelStr = "note: ";
				stream = &std::cout;
				break;
			default:
				continue;
		}

		if (message.continued)
			levelStr = continueStr;

		// Try to emulate the formatting of the host compiler.
		if (!message.file.empty())
		{
			*stream << message.file;
			if (message.line > 0)
			{
#if MSL_MSC
				*stream << "(" << message.line;
				if (message.column > 0)
					*stream << "," << message.column;
				*stream << ")";
#else
				*stream << ":" << message.line;
				if (message.column > 0)
					*stream << ":" << message.column;
#endif
			}

			*stream << ": ";
		}

		*stream << levelStr << message.message << std::endl;
	}
}

int main(int argc, char** argv)
{
	// Specify the options.
	options_description mainOptions("main options");
	mainOptions.add_options()
		("help,h", "display this help message")
		("config,c", value<std::string>()->required(), "configuration file describing the target")
		("input,i", value<std::vector<std::string>>()->required(), "input file to compile. "
			"Multiple inputs may be provided to compile into a single module.")
		("output,o", value<std::string>()->required(), "output file for the compiled result")
		("include,I", value<std::vector<std::string>>(), "directory to search for includes")
		("define,D", value<std::vector<std::string>>(), "add a define for the preprocessor. A "
			"value may optionally be assigned with =. (i.e. -D DEFINE=val)")
		("warn-none,w", "disable all warnings")
		("warn-error,W", "treat warnings as errors")
		("strip,s", "strip debug symbols")
		("optimize,O", "optimize the compiled result");

	options_description configOptions("options in target configuration file");
	configOptions.add_options()
		("target", value<std::string>()->required(), "the target to compile for. "
			"Possible values are: spirv, glsl, glsl-es, metal-osx, metal-ios")
		("version", value<std::string>(), "the version of the target. Required for GLSL and "
			"Metal.")
		("define", value<std::vector<std::string>>(), "add a define for the preprocessor. A value "
			"may optionally be assigned with =. (i.e. DEFINE=val)")
		("force-enable", value<std::vector<std::string>>(), "force a feature to be enabled")
		("force-disable", value<std::vector<std::string>>(), "force a feature to be disabled")
		("resources", value<std::string>(), "a path to a file describing custom resource limits. "
			"This uses the same format as glslangValidator.")
		("spirv-command", value<std::string>(), "external command to run on the intermediate "
			"SPIR-V. The string $input will be replaced by the input file path, while the string "
			"$output will be replaced by the output file path.")
		("remap-variables", value<bool>(), "remap variable ranges to improve compression of SPIR-V")
		("dummy-bindings", value<bool>(), "add dummy bindings in SPIR-V to be changed later")
		("adjustable-bindings", value<bool>(), "allow uniform bindings to be adjusted in-place "
			"with SPIR-V; this also enables dummy-bindings")
		("remap-depth-range", value<bool>(), "boolean for whether or not to remap the depth range "
			"from [0, 1] to [-1, 1] in the  vertex shader output for GLSL or Metal targets. "
			"Defaults to false.")
		("default-float-precision", value<std::string>(), "the default precision to use for "
			"floats in GLSL targets. Possible values are: none, low, medium, high. Defaults to "
			"medium.")
		("default-int-precision", value<std::string>(), "the default precision to use for ints in "
			"in GLSL targets. Possible values are: none, low, medium, high. Defaults to high.")
		("pre-header-line", value<std::vector<std::string>>(), "header line to be added verbatim "
			"before any processing.")
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

	// Parse the options.
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

	bool printHelp = options.count("help") > 0;

	// Parse the config file.
	variables_map config;
	std::string configFilePath;
	if (exitCode == 0 && !printHelp)
	{
		configFilePath = options["config"].as<std::string>();
		try
		{
			store(parse_config_file<char>(configFilePath.c_str(), configOptions), config);
			notify(config);
		}
		catch (std::exception& e)
		{
			std::cerr << configFilePath << " error: " << e.what() << std::endl << std::endl;
			exitCode = 1;
		}
	}

	// Create the target and set the options.
	std::unique_ptr<msl::Target> target;
	if (!printHelp && exitCode == 0)
	{
		std::string targetName = config["target"].as<std::string>();
		if (targetName == "spirv")
			target.reset(new msl::TargetSpirV);
		else if (targetName == "glsl" || targetName== "glsl-es")
			target = createGlslTarget(targetName, config, configFilePath);
		else if (targetName == "metal-osx" || targetName == "metal-ios")
			target = createMetalTarget(targetName, config, configFilePath);
		else
		{
			std::cerr << "error: unkown target: " << targetName << std::endl << std::endl;
			exitCode = 1;
		}

		if (!target || !setCommonTargetConfig(*target, options, config, configFilePath))
			exitCode = 1;
	}

	if (printHelp || exitCode != 0)
	{
		std::cout << "Usage: mslc [options] -c config -o output file1 [file2...]" << std::endl <<
			std::endl;
		std::cout << "Compile one or more shader source files into a shader module." << std::endl <<
			std::endl;
		std::cout <<
			"In order to determine how to compile the shader, a target configuration file\n"
			"must be provided. This configuration file takes the form of name/value pairs.\n"
			"For example:\n"
			"    target = glsl-es\n"
			"    version = 300\n"
			"    force-disable = UniformBlocks\n"
			"    force-disable = Derivatives\n"
			"    remap-depth-range = yes" << std::endl << std::endl;
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

	msl::Output output;
	msl::CompiledResult result;
	for (const std::string& input : options["input"].as<std::vector<std::string>>())
	{
		if (!target->compile(result, output, input))
		{
			exitCode = 2;
			break;
		}
	}

	if (exitCode == 0)
	{
		if (!target->finish(result, output))
			exitCode = 2;
	}

	if (output.getErrorCount() > 0)
		exitCode = 2;

	printOutput(output, options.count("warn-none") == 0);
	if (options.count("warn-error") && output.getWarningCount() > 0)
	{
		std::cerr << "error: warnings treated as errors" << std::endl;
		exitCode = 3;
	}

	if (exitCode != 0)
	{
		std::cerr << "error: compilation failed" << std::endl;
		return exitCode;
	}

	std::string outputFile = options["output"].as<std::string>();
	if (!result.save(outputFile))
	{
		std::cerr << "error: could not write output file: " << outputFile << std::endl;
		return 4;
	}

	std::cout << "output shader module to " << outputFile << std::endl;
	return exitCode;
}
