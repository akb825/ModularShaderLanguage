if (NOT GTEST_FOUND OR NOT MSL_BUILD_TESTS)
	return()
endif()

find_package(Boost COMPONENTS filesystem)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(msl_client_test ${sources})

add_custom_command(TARGET msl_client_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy
	${CMAKE_CURRENT_SOURCE_DIR}/CompleteShader.mslb $<TARGET_FILE_DIR:msl_client_test>
	COMMENT "Copying test shader module." VERBATIM)

target_include_directories(msl_client_test PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(msl_client_test
	PRIVATE msl_client ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES})

set_folder(msl_client_test tests)
add_test(NAME MSLClientTest COMMAND msl_client_test)
