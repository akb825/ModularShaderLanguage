# Introduction

The Modular Shader Language (MSL) is a pre-compiled shader language with multiple targets. It is effectively a wrapper around GLSL and relies on external tools to handle the main work of the compilation. The primary goals are:

* Provide an environment more similar to other compiled languages, such as access to a preprocessor and compile-time errors that point to the file.
* Allow targetting multiple platforms with different capabilities with the same source.
* Relies on official and external tools as much as possible. This makes the language easier to extend and maintain and allows other third party tools, such as optimizers, to be used.
* Allows all stages of the pipeline to be specified in the same source. This gives more flexibility in how you organize your source.
* Pipeline is linked when compiling the shader modules, allowing for earlier checks and easier loading of shaders in the final application.

# Language and compilation overview

The core of the language is GLSL 450 for Vulkan with the following changes:

* Uses a C preprocessor to allow for advanced macros and \#includes.
* Allows tagging of elements to only be included for specific pipeline stages of the shader.
* Allows the removal of uniform blocks to use the same source on targets that don't support them.
* Declaration of the full pipeline used with the entry points used at each stage. Multiple pipelines can be declared in the same shader. (e.g. different rendering techniques or passes)

Not all language features will be available on all targets. The targets will pre-define macros to determine their capabilities, allowing for conditional compiling can be used to switch between implementations when differences arise.

After the initial processing is done on the shader to transform it into standard GLSL, it gets passed into [glslang](https://github.com/KhronosGroup/glslang) to convert it to SPIR-V. From there, [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) is used to convert from SPIR-V to other targets, including various flavors of GLSL (including ES) and Metal. (HLSL support will hopefully come later)

After translating the individual entry points to the target language, it may additionally go through an additional tool to process the file. For example, another shader compiler (such as the HLSL or Metal compilers) or an optimizer. (such as GLSL Optimizer)

Multiple shader files may be compiled into a module. Examples for combining modules include:

* All of the render passes, quality levels, etc. for a single object.
* Separate modules for each quality level.
* All of the shaders for a specific library.

# Dependencies

The following software is required to build DeepSea:

* [cmake](https://cmake.org/) 3.0.2 or later
* [boost](http://www.boost.org/) (required for compiler)
* [glslang](https://github.com/KhronosGroup/glslang) (required for compiler, provided as submodule)
* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) (required for compiler, provided as submodule)
* [doxygen](http://www.stack.nl/~dimitri/doxygen/) (optional)
* [gtest](https://github.com/google/googletest) (optional)

The glslang and SPIRV-Cross submodules can be grabbed by running the commands

	ModularShaderLanguage$ git submodule init
	ModularShaderLanguage$ git submodule update

Additionally, additional tools such as Microsoft's HLSL compiler and Apple's Metal compiler will be required when compiling shaders for certain platforms.

# Platforms

MSL has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)

# Building

Building is done with CMake. To build a release build, execute the following commands:

	ModularShaderLanguage$ mkdir build
	ModularShaderLanguage$ cd build
	ModularShaderLanguage/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	ModularShaderLanguage/build$ make

The following options may be used when running cmake:

## Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running make install.
* `-DMSL_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `OFF`.

## Enabled Builds

* `-DMSL_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DMSL_BUILD_DOCS=ON|OFF`: Set to `ON` to build the documentation. `doxygen` must also be found in order to build the documentation. Defaults to `ON`.
* `-DMSL_BUILD_COMPILE=ON|OFF`: Set to `ON` to build the compiler. Defaults to `ON`.
* `-DMSL_BUILD_TOOLS=ON|OFF`: Set to `ON` to build the tools. Defaults to `ON`.

## Miscellaneous Options:

* `-DMSL_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DMSL_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding MSL in another project. Defaults to MSL.

Once you have built and installed MSL, and have added the `lib/cmake/MSL` directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

    find_package(MSL MODULES Compile)

Libraries and include directories can be found through the `MSLModule_LIBRARIES` and `MSLModule_INCLUDE_DIRS` CMake variables. For example: `MSLCompile_LIBRARIES` and `MSLCompile_INCLUDE_DIRS`.

# Modules

MSL contains the following modules:

* [Compile](Compile/README.md): (Optional) The library for processing shader files, feeding them to the various tools, and outputting the final files.
* [tools](tools/README.md): (Optional) The `mslc` tool for compiling shader files.

The directory structure of the include files is:

	MSL/[Module]/Header.h

For example:

	#include <MSL/Compile/Target.h>

