file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h include/*.hpp)

add_library(msl_client ${MSL_LIB} ${sources})
add_dependencies(msl_client mslb)

target_include_directories(msl_client PRIVATE ${FLATBUFFERS_INCLUDE_DIRS} ${SHARED_DIR})

msl_set_folder(msl_client libs)
msl_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/MSL/Client
	FILES ${sources})

msl_install_library(TARGET msl_client MODULE Client)
set(MSL_DOC_PROJECTS ${MSL_DOC_PROJECTS} Client PARENT_SCOPE)

add_subdirectory(test)
