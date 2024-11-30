find_package(Boost CONFIG COMPONENTS filesystem wave REQUIRED)
find_package(Threads)

set(GLSLANG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glslang)
set(SPIRV_CROSS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Cross)
set(SPIRV_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Tools)
if (NOT EXISTS ${GLSLANG_DIR} OR NOT EXISTS ${SPIRV_CROSS_DIR} OR NOT EXISTS ${SPIRV_TOOLS_DIR})
	message(FATAL_ERROR
		"glslang, SPIRV-Cross, or SPIRV-Tools not found. Run 'git submodule update --init' to pull "
		"the submodules.")
endif()

file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)

# Add glslang sources.
# We cannot use the cmake files that come with these projects because they don't use cmake exports.
if (WIN32)
	add_definitions(-DGLSLANG_OSINCLUDE_WIN32)
	set(glslangOsSources ${GLSLANG_DIR}/glslang/OSDependent/Windows/ossource.cpp)
elseif (UNIX)
	add_definitions(-DGLSLANG_OSINCLUDE_UNIX)
	set(glslangOsSources ${GLSLANG_DIR}/glslang/OSDependent/Unix/ossource.cpp)
else()
	message("unknown platform")
endif()

file(GLOB_RECURSE glslangSources
	${GLSLANG_DIR}/glslang/MachineIndependent/*.cpp ${GLSLANG_DIR}/glslang/GenericCodeGen/*.cpp
	${GLSLANG_DIR}/glslang/Public/*.h ${GLSLANG_DIR}/glslang/Include/*.h
	${GLSLANG_DIR}/glslang/ResourceLimits/*.cpp ${GLSLANG_DIR}/glslang/ResourceLimits/*.h
	${GLSLANG_DIR}/OGLCompilersDLL/*.cpp ${GLSLANG_DIR}/OGLCompilersDLL/*.h
	${GLSLANG_DIR}/SPIRV/*.cpp ${GLSLANG_DIR}/SPIRV/*.h)
file(GLOB spirvCrossSources ${SPIRV_CROSS_DIR}/spirv_*.cpp ${SPIRV_CROSS_DIR}/spirv_*.hpp)

# SPIRV-Tools has too complex of a build configuration to just embed the source in this library,
# but the CMakeLists.txt is set up to properly manage library exports. Only need to install the
# libraries if a static build, though.
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ${MSL_SHARED} CACHE BOOL "" FORCE)
# Ignore warnings for SPIRV-Tools.
set(origCxxFlags ${CMAKE_CXX_FLAGS})
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()
add_subdirectory(SPIRV-Tools)
set(CMAKE_CXX_FLAGS ${origCxxFlags})

# Don't care about warnings for external files.
set(externalSources ${glslangSources} ${glslangOsSources} ${spirvCrossSources})
if (MSVC)
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
else()
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS -w)
endif()

add_library(msl_compile ${MSL_LIB} ${sources} ${externalSources})
add_dependencies(msl_compile mslb)

target_include_directories(msl_compile
	PRIVATE ${FLATBUFFERS_INCLUDE_DIRS}
			${GLSLANG_DIR}
			${SPIRV_CROSS_DIR}
			${SHARED_DIR}
			src)
target_link_libraries(msl_compile
	PRIVATE SPIRV-Tools-opt Boost::filesystem Boost::wave ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(msl_compile PRIVATE BOOST_ALL_NO_LIB)

msl_set_folder(msl_compile libs)
msl_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/MSL/Compile
	FILES ${sources})
msl_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} FILES ${externalSources})

if (NOT MSL_SHARED)
    # Only need to include the external dependencies for static library build so it can be linked.
	set(externalDepends EXTERNAL_DEPENDS SPIRV-Tools-opt BOOST_DEPENDS system thread filesystem
		wave)
else()
	set(externalDepends)
endif()
msl_install_library(TARGET msl_compile MODULE Compile ${externalDepends})
set(MSL_DOC_PROJECTS ${MSL_DOC_PROJECTS} Compile PARENT_SCOPE)

add_subdirectory(test)
