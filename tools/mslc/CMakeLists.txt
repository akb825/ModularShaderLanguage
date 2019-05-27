if (NOT MSL_SHARED)
	set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost COMPONENTS system program_options)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(mslc ${sources})

target_include_directories(mslc PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mslc PRIVATE msl_compile ${Boost_LIBRARIES})
target_compile_definitions(mslc PRIVATE MSL_MAJOR_VERSION=${MSL_MAJOR_VERSION}
	MSL_MINOR_VERSION=${MSL_MINOR_VERSION} MSL_PATCH_VERSION=${MSL_PATCH_VERSION})

msl_set_folder(mslc tools)

if (MSL_INSTALL)
	install(TARGETS mslc RUNTIME DESTINATION bin COMPONENT tools)
endif()

set(testPath ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(mslcPath $<TARGET_FILE:mslc>)
if (WIN32)
	set(commandPath ${testPath}/run-test.bat)
else()
	set(commandPath ${testPath}/run-test.sh)
endif()

add_test(NAME MSLCNoArgs
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "" 1)
add_test(NAME MSLCHelp
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-h" 0)
add_test(NAME MSLCNoConfig
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-o test.mslb shaders/CompleteShader.msl" 1)
add_test(NAME MSLCNoOutput
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf shaders/CompleteShader.msl" 1)
add_test(NAME MSLCNoInput
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf -o test.mslb" 1)
add_test(NAME MSLCInvalidConfig
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c empty.conf -o test.mslb shaders/CompleteShader.msl" 1)
add_test(NAME MSLCCompile
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf -o test.mslb shaders/CompleteShader.msl" 0)
add_test(NAME MSLCCompileGLSL
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl.conf -o test.mslb shaders/CompleteShader.msl" 0)
add_test(NAME MSLCCompileGLSLES
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl-es.conf -o test.mslb shaders/CompleteShader.msl" 0)
add_test(NAME MSLCCompileGLSLOld
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl-old.conf -o test.mslb shaders/CompleteShader.msl" 0)
add_test(NAME MSLCCompileDuplicate
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf -o test.mslb shaders/CompleteShader.msl shaders/CompileWarning.msl" 2)
add_test(NAME MSLCInvalidOutput
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf -o invalid/test.mslb shaders/CompleteShader.msl" 4)
add_test(NAME MSLCCompileError
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf -o test.mslb -I shaders shaders/CompileError.msl" 2)
add_test(NAME MSLCCompileWarning
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf -o test.mslb -I shaders shaders/CompileWarning.msl" 0)
add_test(NAME MSLCCompileWarningAsError
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c spirv.conf -o test.mslb -I shaders -W shaders/CompileWarning.msl" 3)
add_test(NAME MSLCGlsl
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 0)
add_test(NAME MSLCGlslDisable
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl-disable.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 0)
add_test(NAME MSLCGlslEs
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl-es.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 0)

if (APPLE)
	add_test(NAME MSLCMetalOsX
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${mslcPath} "-c metal-osx.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 0)
	add_test(NAME MSLCMetalIos
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${mslcPath} "-c metal-ios.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 0)
endif()

add_test(NAME MSLCGlslNoVersion
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl-no-version.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 1)
add_test(NAME MSLCGlslBadVersion
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c glsl-bad-version.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 1)
add_test(NAME MSLCMetalNoVersion
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c metal-osx-no-version.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 1)
add_test(NAME MSLCMetalBadVersion
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${mslcPath} "-c metal-osx-bad-version.conf -o test.mslb -I shaders -D COMMAND_LINE_DEFINE=1 shaders/Defines.msl" 1)
