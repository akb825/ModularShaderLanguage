if (NOT MSL_SHARED)
	set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost COMPONENTS system program_options filesystem)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(mslb-extract ${sources})

target_include_directories(mslb-extract PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mslb-extract PRIVATE msl_client ${Boost_LIBRARIES})

msl_set_folder(mslb-extract tools)

install(TARGETS mslb-extract RUNTIME DESTINATION bin COMPONENT tools)
