cmake_minimum_required(VERSION 3.5)
project(MSL)

# Build options
# Use if since BUILD_SHARED_LIBS defaults to unset.
if (BUILD_SHARED_LIBS)
	set(MSL_SHARED_DEFAULT ON)
else()
	set(MSL_SHARED_DEFAULT OFF)
endif()
set(MSL_SHARED ${MSL_SHARED_DEFAULT} CACHE BOOL "Build MSL using shared libraries.")

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
set(MSL_INSTALL_SET_RPATH ON CACHE BOOL "Set rpath for libraries and tools on installation.")

if (NOT MSL_BUILD_COMPILE AND NOT MSL_BUILD_CLIENT)
	message(FATAL_ERROR
		"Must have at least one of MSL_BUILD_COMPILE and MSL_BUILD_CLIENT set to ON.")
endif()

if (APPLE AND NOT IOS AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS deployment version." FORCE)
endif()

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

# Find flatc and the flatbuffer includes. Only use system flatbuffers and automatic generation if
# both are found to avoid incompatibilities.
find_program(FLATC flatc)
find_path(SYSTEM_FLATBUFFERS_INCLUDE_DIRS flatbuffers/flatbuffers.h)
if (FLATC AND SYSTEM_FLATBUFFERS_INCLUDE_DIRS)
	set(FLATBUFFERS_INCLUDE_DIRS ${SYSTEM_FLATBUFFERS_INCLUDE_DIRS})
	set(MSL_BUILD_FLATBUFFERS ON)
else()
	set(FLATBUFFERS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Shared/external)
	set(MSL_BUILD_FLATBUFFERS OFF)
	message("flatc not installed. Using pre-generated flatbuffers.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSL_SHARED)
	set(MSL_LIB SHARED)
	set(Boost_USE_STATIC_LIBS OFF)
else()
	set(MSL_LIB STATIC)
	set(Boost_USE_STATIC_LIBS ON)
endif()

if (MSL_OUTPUT_DIR)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MSL_OUTPUT_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MSL_OUTPUT_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MSL_OUTPUT_DIR})
endif()

set(MSL_MAJOR_VERSION 1)
set(MSL_MINOR_VERSION 7)
set(MSL_PATCH_VERSION 2)
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
