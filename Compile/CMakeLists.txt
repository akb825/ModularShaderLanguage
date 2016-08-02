if (NOT MSL_SHARED)
	set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost COMPONENTS wave)
find_package(Threads)

set(GLSLANG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glslang)
set(SPIRV_CROSS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Cross)
if (NOT EXISTS ${GLSLANG_DIR} OR NOT EXISTS ${SPIRV_CROSS_DIR})
	message(FATAL_ERROR
		"glslang or SPIRV-Cross not found. Run 'git submodule init' and "
		"'git submodule update' to pull the submodules.")
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
	${GLSLANG_DIR}/OGLCompilersDLL/*.cpp ${GLSLANG_DIR}/OGLCompilersDLL/*.h
	${GLSLANG_DIR}/SPIRV/*.cpp ${GLSLANG_DIR}/SPIRV/*.h
	${GLSLANG_DIR}/hlsl/*.cpp ${GLSLANG_DIR}/hlsl/*.h)
set(glslResourceSources
	${GLSLANG_DIR}/StandAlone/ResourceLimits.cpp ${GLSLANG_DIR}/StandAlone/ResourceLimits.h)
file(GLOB spirvCrossSources ${SPIRV_CROSS_DIR}/spirv_*.cpp ${SPIRV_CROSS_DIR}/spirv_*.hpp)
set(externalSources
	${glslangSources} ${glslangOsSources} ${glslResourceSources} ${spirvCrossSources})

# Don't care about warnings for external files.
if (MSVC)
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
else()
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS -w)
endif()

add_library(msl_compile ${MSL_LIB} ${sources} ${externalSources})

target_include_directories(msl_compile
	PRIVATE ${Boost_INCLUDE_DIRS} ${GLSLANG_DIR} ${SPIRV_CROSS_DIR} src)
target_link_libraries(msl_compile
	PRIVATE ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_folder(msl_compile libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/MSL/Compile
	FILES ${sources})
setup_filters(SRC_DIR . FILES ${externalSources})

install_library(TARGET msl_compile MODULE Compile)
set(MSL_DOC_PROJECTS ${MSL_DOC_PROJECTS} Compile PARENT_SCOPE)

add_subdirectory(test)
