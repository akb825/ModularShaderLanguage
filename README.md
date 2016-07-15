# Introduction

The Modular Shader Language (MSL) is a pre-compiled shader language with multiple targets. Current and planned targets are:

* GLSL (120, 330, 400, 430, 100ES, 300ES, 310ES) (TODO)
* SPIR-V (TODO)
* HLSL (shader models 3, 4, 4.1, and 5) (TODO)
* Metal (TODO)

Not all language features will be available on all targets. Conditional compiling can be used to switch between implementations when differences arise.

External tools can be used to do part of the compilation. For example, Microsoft's HLSL compiler and Apple's Metal compiler. This ensures that the resulting binary is optimal for those targets. The generated high-level code for the high level targets is kept as close as possible to the original code to make it easier to debug.

In the case of SPIR-V, some optimizations such as dead code elimination, constant propagation, and elimination of trivial if statements may be performed. However, as recommended by Kronos, more aggressive optimizations are left to the driver. Debugging information may be embedded within SPIR-V.

Shaders can be compiled into modules. This will contain many entry points and a group of shared resources, such as buffers. Common examples include:

* All the pipeline stages for a single set of shaders from vertex through fragment.
* All the shaders for different ways of rendering a type of object.
* All the shaders used in a particular library.
* Separate modules containing all the shaders for each quality setting.
* All of the shaders within your application.

# Dependencies

The following software is required to build DeepSea:

* cmake 3.0.2 or later
* doxygen (optional)
* gtest (optional)
* flex and flex (required if making modifications to the parser)

Additionally, additional tools such as Microsoft's HLSL compiler and Apple's Metal compiler will be required when compiling shaders for certain platforms. glslang may also be used as a sanity check for GLSL output.

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
* `-DMSL_BUILD_BACKEND=ON|OFF`: Set to `ON` to build the backend. When set to `OFF`, only the frontend will be built. Defaults to `ON`.
* `-DMSL_BUILD_TARGETS=...`: A semicolon-separated list of the targets to build for. Possible options are: `GLSL`, `SPIR-V`, `HLSL`, `METAL`, `ALL`, and `NONE`. `ALL` will use all targets available on the current platform. Defaults to `ALL`.
* `-DMSL_BUILD_TOOLS=ON|OFF`: Set to `ON` to build the tools. Defaults to `ON`.

## Miscellaneous Options:

* `-DMSL_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DMSL_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding MSL in another project. Defaults to MSL.

Once you have built and installed MSL, and have added the `lib/cmake/MSL` directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

    find_package(MSL MODULES Frontend Backend)

Libraries and include directories can be found through the `MSL_LIBRARIES` and `MSLModule_NCLUDE_DIRS` CMake variables. For example: `MSLFrontend_LIBRARIES` and `MSLFrontend_INCLUDE_DIRS`.

# Modules

MSL contains the following modules:

* [Frontend](Frontend/README.md): (Required) The frontend for parsing and analyzing MSL source files. This may be embedded in other applications for features such as syntax and symantic highlighting, code completion, etc.
* [Backend](Backend/README.md): (Optional) Converts the parsed shader to various targets.
* [tools](tools/README.md): (Optional) Tools for compiling shaders and verifying that certain shaders can be linked together at runtime.

The directory structure of the include files is:

	MSL/<ModuleName>/[Subdirectory/]Header.h

For example:

	#include <MSL/Frontend/Config.h>
	#include <MSL/Frontend/Parse/Lexer.h>
