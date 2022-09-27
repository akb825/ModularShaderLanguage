if (NOT GTEST_FOUND OR NOT MSL_BUILD_TESTS)
	return()
endif()

find_package(Boost COMPONENTS filesystem REQUIRED)
find_package(Threads)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(msl_client_test ${sources})

add_custom_command(TARGET msl_client_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy
	${CMAKE_CURRENT_SOURCE_DIR}/CompleteShader.mslb ${CMAKE_CURRENT_SOURCE_DIR}/ComputeShader.mslb
	$<TARGET_FILE_DIR:msl_client_test>
	COMMENT "Copying test shader module." VERBATIM)

target_include_directories(msl_client_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(msl_client_test
	PRIVATE MSL::Client Boost::filesystem ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

msl_set_folder(msl_client_test tests)
add_test(NAME MSLClientTest COMMAND msl_client_test)
