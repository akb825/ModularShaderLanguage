cmake_minimum_required(VERSION 3.1)
project(MSL)

# Build options
set(MSL_SHARED OFF CACHE BOOL "Build MSL using shared libraries.")

# Options for disabling portions of the build.
set(MSL_BUILD_TESTS ON CACHE BOOL "Build unit tests.")
set(MSL_BUILD_DOCS ON CACHE BOOL "Build documentation.")

set(MSL_BUILD_COMPILE ON CACHE BOOL "Build the compile library.")
set(MSL_BUILD_CLIENT ON CACHE BOOL "Build the client library.")
set(MSL_BUILD_TOOLS ON CACHE BOOL "Build the tools.")

# Misc options.
set(MSL_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output CACHE PATH "Folder for placing the build output.")
set(MSL_EXPORTS_DIR ${CMAKE_BINARY_DIR}/cmake CACHE PATH
	"Folder for placing the cmake exports while building. Useful when embedding in other projects.")
set(MSL_ROOT_FOLDER MSL CACHE STRING
	"Root folder for the MSL projects. Usefull when embedding in other projects.")
set(MSL_INSTALL ON CACHE BOOL "Allow installation for MSL components.")

if (MSL_BUILD_TESTS)
	find_package(GTest QUIET)
	if (NOT GTEST_FOUND)
		message("GTest not installed. Skipping tests.")
	endif()
endif()

if (MSL_BUILD_DOCS)
	find_package(Doxygen QUIET)
	if (NOT DOXYGEN_FOUND)
		message("Doxygon not installed. Skipping documentation.")
	endif()
endif()

# This will try to find the include directory on the system, and fall back to the version checked
# into the repository.
find_path(FLATBUFFERS_INCLUDE_DIRS flatbuffers/flatbuffers.h)
if (NOT FLATBUFFERS_INCLUDE_DIRS)
	set(FLATBUFFERS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Shared/external CACHE PATH
		"FlatBuffers include directory" FORCE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSL_SHARED)
	set(MSL_LIB SHARED)
else()
	set(MSL_LIB STATIC)
endif()

if (MSL_OUTPUT_DIR)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MSL_OUTPUT_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MSL_OUTPUT_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MSL_OUTPUT_DIR})
endif()

set(MSL_MAJOR_VERSION 1)
set(MSL_MINOR_VERSION 4)
set(MSL_PATCH_VERSION 3)
set(MSL_VERSION ${MSL_MAJOR_VERSION}.${MSL_MINOR_VERSION}.${MSL_PATCH_VERSION})

set(MSL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/config.cmake)
include(cmake/install.cmake)

add_subdirectory(Shared)
set(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shared)

if (MSL_BUILD_COMPILE)
	add_subdirectory(Compile)
endif()

if (MSL_BUILD_CLIENT)
	add_subdirectory(Client)
endif()

if (MSL_BUILD_TOOLS)
	add_subdirectory(tools)
endif()

# Documentation. (populated by built libraries)
add_subdirectory(doc)

msl_install_master_config()
