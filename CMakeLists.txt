cmake_minimum_required(VERSION 3.0.2)
project(MSL)
	
# Build options
set(MSL_SHARED OFF CACHE BOOL "Build MSL using shared libraries.")
set(MSL_PROFILING ON CACHE BOOL "Enable profiling of code.")

# Options for disabling portions of the build.
set(MSL_BUILD_TESTS ON CACHE BOOL "Build unit tests.")
set(MSL_BUILD_DOCS ON CACHE BOOL "Build documentation.")

set(MSL_BUILD_COMPILE ON CACHE BOOL "Build the compiler.")
set(MSL_BUILD_TOOLS ON CACHE BOOL "Build tools.")

# Misc options.
set(MSL_EXPORTS_DIR ${CMAKE_BINARY_DIR}/cmake CACHE PATH
	"Folder for placing the cmake exports while building. Useful when embedding in other projects.")
set(MSL_ROOT_FOLDER MSL CACHE STRING
	"Root folder for the MSL projects. Usefull when embedding in other projects.")
	
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

find_file(FLATBUFFERS_H flatbuffers/flatbuffers.h)
if (NOT FLATBUFFERS_H)
	set(FLATBUFFERS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Shared/external)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSL_SHARED)
	set(MSL_LIB SHARED)
else()
	set(MSL_LIB STATIC)
endif()

set(MSL_MAJOR_VERSION 0)
set(MSL_MINOR_VERSION 0)
set(MSL_PATCH_VERSION 1)
set(MSL_VERSION ${MSL_MAJOR_VERSION}.${MSL_MINOR_VERSION}.${MSL_PATCH_VERSION})

include(cmake/config.cmake)
include(cmake/install.cmake)

add_subdirectory(Shared)
set(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shared)

if (MSL_BUILD_COMPILE)
	add_subdirectory(Compile)
endif()

# Documentation. (populated by built libraries)
add_subdirectory(doc)

install_master_config()
