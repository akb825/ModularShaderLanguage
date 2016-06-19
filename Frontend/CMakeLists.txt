file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)
add_library(msl_frontend ${MSL_LIB} ${sources})

set_folder(msl_frontend libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/MSL/Frontend
	FILES ${sources})

install_library(TARGET msl_frontend MODULE Frontend)
set(MSL_DOC_PROJECTS ${MSL_DOC_PROJECTS} Frontend PARENT_SCOPE)

add_subdirectory(test)
