if (NOT MSL_SHARED)
	set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost COMPONENTS program_options)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(mslc ${sources})

target_include_directories(mslc PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mslc PRIVATE msl_compile ${Boost_LIBRARIES})

set_folder(mslc tools)
