# Introduction

The Modular Shader Language (MSL) is a pre-compiled shader language with multiple targets. Current and planned targets are:

* GLSL (TODO)
	* OpenGL 2.1, 3.3, and 4.5
	* OpenGL ES 2.0, 3.0, 3.1
* SPIR-V (TODO)
* Direct3D (TODO)
	* Shader models 3, 4, and 5.
* Metal (TODO)

Not all language features will be available on all targets. Conditional compiling can be used to switch between implementations when differences arise.

Compiling straight to bytecode is preferred. Based on the target, the final result will be one of the following, from most to least preferred:

* Directly to bytecode.
* Intermediate compile to a high level to pass to an external tool. This is done when the bytecode format isn't publically available.
* To a high level language to be compiled by the driver at runtime.

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
* LLVM 3.8 or later (not required when only building frontend)
* Metal compiler when building Metal target

# Platforms

DeepSea has been built for and tested on the following platforms:

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
* `-DMSL_BUILD_TARGETS=...`: A semicolon-separated list of the targets to build for. Possible options are: `GLSL`, `SPIR-V`, `D3D`, `METAL`, `ALL`, and `NONE`. `ALL` will use all targets available on the current platform. Defaults to `ALL`.
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
* [Backend](Backend/README.md): (Optional) Converts the parsed shader to LLVM-IR and performs transformations through LLVM, such as optimizations.
* [Targets](Targets/README.md): (Optional) Targets for the final compiled shaders.
* [tools](tools/README.md): (Optional) Tools for compiling shaders and verifying that certain shaders can be linked together at runtime.

The directory structure of the include files is:

	MSL/<ModuleName>/[Subdirectory/]Header.h

For example:

	#include <MSL/Frontend/Config.h>
	#include <MSL/Frontend/Parse/Lexer.h>
