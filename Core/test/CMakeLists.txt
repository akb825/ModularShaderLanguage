if (NOT GTEST_FOUND OR NOT MSL_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(msl_core_test ${sources})

add_custom_command(TARGET msl_core_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/inputs $<TARGET_FILE_DIR:msl_core_test>/inputs
	COMMENT "Copying test inputs." VERBATIM)
add_custom_command(TARGET msl_core_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/outputs $<TARGET_FILE_DIR:msl_core_test>/outputs
	COMMENT "Copying test outputs." VERBATIM)

target_include_directories(msl_core_test PRIVATE ${GTEST_INCLUDE_DIRS} ../src)
target_link_libraries(msl_core_test PRIVATE
	msl_core
	${GTEST_BOTH_LIBRARIES})

set_folder(msl_core_test tests)
add_test(NAME MSLCoreTest COMMAND msl_core_test)
