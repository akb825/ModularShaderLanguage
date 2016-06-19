if (NOT GTEST_FOUND OR NOT MSL_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(msl_frontend_test ${sources})

target_include_directories(msl_frontend_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(msl_frontend_test PRIVATE
	msl_frontend
	${GTEST_BOTH_LIBRARIES})

set_folder(msl_frontend_test tests)
add_test(NAME MSLFrontendTest COMMAND msl_frontend_test)
