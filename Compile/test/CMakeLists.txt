if (NOT GTEST_FOUND OR NOT MSL_BUILD_TESTS)
	return()
endif()

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(msl_compile_test ${sources})

add_custom_command(TARGET msl_compile_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/inputs $<TARGET_FILE_DIR:msl_compile_test>/inputs
	COMMENT "Copying test inputs." VERBATIM)
add_custom_command(TARGET msl_compile_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/outputs $<TARGET_FILE_DIR:msl_compile_test>/outputs
	COMMENT "Copying test outputs." VERBATIM)

target_include_directories(msl_compile_test
	PRIVATE ${GTEST_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${GLSLANG_DIR} ${SPIRV_CROSS_DIR} ../src)
target_link_libraries(msl_compile_test
	PRIVATE msl_compile ${Boost_LIBRARIES} ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(msl_compile_test PRIVATE BOOST_ALL_NO_LIB)

msl_set_folder(msl_compile_test tests)
add_test(NAME MSLCompileTest COMMAND msl_compile_test)
