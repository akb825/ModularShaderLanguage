# Introduction

The Modular Shader Language (MSL) is a pre-compiled shader language with multiple targets. It is effectively a wrapper around GLSL and relies on external tools to handle the main work of the compilation. The primary goals are:

* Provide an environment more similar to other compiled languages, such as access to a preprocessor and compile-time errors that point to the file.
* Allow targeting multiple platforms with different capabilities with the same source.
* Relies on official and external tools as much as possible. This makes the language easier to extend and maintain and allows other third party tools, such as optimizers, to be used.
* Allows all stages of the pipeline to be specified in the same source. This gives more flexibility in how you organize your source.
* Allows for render states and sampler states to be declared in the shader. This gives more flexibility in client applications by allowing these to be set in the shader without hard-coding them in the application.
* Pipeline is linked when compiling the shader modules, allowing for earlier checks and easier loading of shaders in the final application.

# Language and compilation overview

See the [Language description](doc/Language.md) for a more detailed description.

The core of the language is GLSL 450 for Vulkan with the following changes:

* Uses a C preprocessor to allow for advanced macros and \#includes.
* Allows tagging of elements to only be included for specific pipeline stages of the shader.
* Allows the removal of uniform blocks to use the same source on targets that don't support them.
* Declaration of the full pipeline used with the entry points used at each stage and render states. Multiple pipelines can be declared in the same shader. (e.g. different rendering techniques or passes)
* Declaration of sampler states.

Not all language features will be available on all targets. The targets will pre-define macros to determine their capabilities, allowing for conditional compiling can be used to switch between implementations when differences arise.

After the initial processing is done on the shader to transform it into standard GLSL, it gets passed into [glslang](https://github.com/KhronosGroup/glslang) to convert it to SPIR-V. From there, [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) is used to convert from SPIR-V to other targets, including various flavors of GLSL (including ES) and Metal.

After translating the individual entry points to the target language, it may additionally go through an additional tool to process the file. For example, another shader compiler (such as the HLSL or Metal compilers) or an optimizer. (such as GLSL Optimizer)

Multiple shader files may be compiled into a module. Examples for combining modules include:

* All of the render passes, quality levels, etc. for a single object.
* Separate modules for each quality level.
* All of the shaders for a specific library.

# Dependencies

The following software is required to build MSL:

* [cmake](https://cmake.org/) 3.1 or later
* [boost](http://www.boost.org/) (required unless only building client library without tests)
* [glslang](https://github.com/KhronosGroup/glslang) (required for compiler, provided as submodule)
* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) (required for compiler, provided as submodule)
* [FlatBuffers](https://google.github.io/flatbuffers/) (required if changing the schema)
* [doxygen](http://www.stack.nl/~dimitri/doxygen/) (optional)
* [gtest](https://github.com/google/googletest) (optional)

> **Note:** Boost must be built with C++11 support. For example, when building and installing through the b2 bootstrap command: `./b2 "-std=c++11" -j4 install`

> **Note:** When Boost is manually installed, the appropriate variables should be set. In the case of Windows, the `BOOST_LIBRARYDIR` and `BOOST_ROOT` variables should be set. (examples: `BOOST_LIBRARYDIR=C:\local\boost_1_64_0\lib64-msvc-14.1` and `BOOST_ROOT=C:\local\boost_1_64_0`) In Windows, the value of `BOOST_LIBRARYDIR` should also be on `PATH` to ensure the DLLs can be loaded.

The glslang and SPIRV-Cross submodules can be grabbed by running the commands

	ModularShaderLanguage$ git submodule init
	ModularShaderLanguage$ git submodule update

Additionally, additional tools such as Microsoft's HLSL compiler and Apple's Metal compiler will be required when compiling shaders for certain platforms.

# Platforms

MSL has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)
* Mac OS X

# Building

Building is done with CMake. To build a release build, execute the following commands:

	ModularShaderLanguage$ mkdir build
	ModularShaderLanguage$ cd build
	ModularShaderLanguage/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	ModularShaderLanguage/build$ make

The tests can be run by running the command:

	ModularShaderLanguage/build$ ctest

The following options may be used when running cmake:

## Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running make install.
* `-DMSL_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `OFF`.

## Enabled Builds

* `-DMSL_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DMSL_BUILD_DOCS=ON|OFF`: Set to `ON` to build the documentation. `doxygen` must also be found in order to build the documentation. Defaults to `ON`.
* `-DMSL_BUILD_COMPILE=ON|OFF`: Set to `ON` to build the compile library. Defaults to `ON`.
* `-DMSL_BUILD_CLIENT=ON|OFF`: Set to `ON` to build the client library. Defaults to `ON`.
* `-DMSL_BUILD_TOOLS=ON|OFF`: Set to `ON` to build the tools. Defaults to `ON`.

## Miscellaneous Options:

* `-DMSL_OUTPUT_DIR=directory`: The folder to place the output files. This may be explicitly left empty in order to output to the defaults used by cmake, but this may prevent tests and executables from running properly when `MSL_SHARED` is set to `ON`. Defaults to `${CMAKE_BINARY_DIR}/output`.
* `-DMSL_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DMSL_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding MSL in another project. Defaults to MSL.
* `-DMSL_INSTALL=ON|OFF`: Allow installation for MSL components. This can be useful when embedding in other projects to prevent installations from including MSL. For example, when statically linking into a shared library. Default is `ON`.

Once you have built and installed MSL, you can find the various modules with the `find_package()` CMake function in `CONFIG` mode. For example:

	find_package(MSL CONFIG COMPONENTS Compile)

Libraries and include directories can be found through the `MSLModule_LIBRARIES` and `MSLModule_INCLUDE_DIRS` CMake variables. For example: `MSLCompile_LIBRARIES` and `MSLCompile_INCLUDE_DIRS`.

> **Note:** In order for `find_package()` to succeed, on Windows you will need to add the path to `INSTALL_DIR/lib/cmake` to `CMAKE_PREFIX_PATH`. (e.g. `C:/Program Files/MSL/lib/cmake`) On other systems, if you don't install to a standard location, you will need to add the base installation path to `CMAKE_PREFIX_PATH`.

# Modules

MSL contains the following modules:

* [Compile](Compile/README.md): (Optional) The library for processing shader files and compiling them into modules.
* [Client](Client/README.md): (Optional) A library for reading in shader modules for C and C++ applications.
* [tools](tools/README.md): (Optional) The `mslc` tool for compiling shader files and `mslb-extract` tool for extracting info from compiled shaders.

The directory structure of the include files is:

	MSL/[Module]/Header.h

For example:

	#include <MSL/Compile/Target.h>

