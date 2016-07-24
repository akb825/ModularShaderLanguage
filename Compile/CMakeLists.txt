if (NOT MSL_SHARED)
	set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost COMPONENTS wave)
find_package(Threads)

file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)
add_library(msl_compile ${MSL_LIB} ${sources})

target_include_directories(msl_compile
	PUBLIC ${Boost_INCLUDE_DIRS}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(msl_compile ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_folder(msl_compile libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/MSL/Compile
	FILES ${sources})

install_library(TARGET msl_compile MODULE Compile)
set(MSL_DOC_PROJECTS ${MSL_DOC_PROJECTS} Compile PARENT_SCOPE)

add_subdirectory(test)
