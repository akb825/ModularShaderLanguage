set(flexSource src/Parse/Lexer.lex)
set(flexOutput src/Parse/Lexer.cpp)

file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)
set(sources ${sources} ${flexSource})
add_library(msl_frontend ${MSL_LIB} ${sources})

target_include_directories(msl_frontend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_program(FLEX_EXE flex)
if (FLEX_EXE)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${flexOutput}
		COMMAND ${FLEX_EXE} ARGS -o ${flexOutput} -R --prefix=msl ${flexSource}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${flexSource}
		COMMENT "Generating lexer..." VERBATIM)
	add_custom_target(msl_flex DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${flexOutput})
	add_dependencies(msl_frontend msl_flex)
else()
	message("flex not found. Using pre-generated lexer.")
endif()

set_folder(msl_frontend libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/MSL/Frontend
	FILES ${sources})

install_library(TARGET msl_frontend MODULE Frontend)
set(MSL_DOC_PROJECTS ${MSL_DOC_PROJECTS} Frontend PARENT_SCOPE)

add_subdirectory(test)
