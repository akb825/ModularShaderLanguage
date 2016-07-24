set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS wave)
find_package(Threads)

file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)
add_library(msl_core ${MSL_LIB} ${sources})

target_include_directories(msl_core
	PUBLIC ${Boost_INCLUDE_DIRS}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(msl_core ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_folder(msl_core libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/MSL
	FILES ${sources})

install_library(TARGET msl_core MODULE Core)
set(MSL_DOC_PROJECTS ${MSL_DOC_PROJECTS} Core PARENT_SCOPE)

add_subdirectory(test)
