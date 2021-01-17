find_package(Boost COMPONENTS system program_options filesystem)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(mslb-extract ${sources})

target_include_directories(mslb-extract PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mslb-extract PRIVATE msl_client ${Boost_LIBRARIES})
target_compile_definitions(mslb-extract PRIVATE BOOST_ALL_NO_LIB
	MSL_MAJOR_VERSION=${MSL_MAJOR_VERSION} MSL_MINOR_VERSION=${MSL_MINOR_VERSION}
	MSL_PATCH_VERSION=${MSL_PATCH_VERSION})

msl_set_folder(mslb-extract tools)

if (MSL_INSTALL)
	install(TARGETS mslb-extract RUNTIME DESTINATION bin COMPONENT tools)
endif()
