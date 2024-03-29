if (NOT DOXYGEN_FOUND OR NOT MSL_BUILD_DOCS)
	return()
endif()

set(rootDir ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(docSource "\"${rootDir}/Shared/include\" \"${rootDir}/README.md\" \"${rootDir}/tools/README.md\" \"${rootDir}/doc/Language.md\"")
set(includeDirs ${rootDir}/Shared/include)
set(dependencies ${rootDir}/Shared/include/MSL/Config.h ${rootDir}/README.md
	${rootDir}/tools/README.md ${rootDir}/doc/Language.md)
foreach (docProject ${MSL_DOC_PROJECTS})
	set(thisDir ${rootDir}/${docProject})
	set(docSource "${docSource} \"${thisDir}/include\" \"${thisDir}/README.md\"")
	set(includeDirs "${includeDirs} \"${thisDir}/include\"")
	file(GLOB_RECURSE theseDependencies ${thisDir}/include/*.h)
	set(dependencies ${dependencies} ${theseDependencies} ${thisDir}/README.md)
endforeach()

configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
set(outputFile ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)
add_custom_command(OUTPUT ${outputFile}
	COMMAND ${DOXYGEN_EXECUTABLE} ARGS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${rootDir}
	DEPENDS ${dependencies} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile ${rootDir}/README.md
	COMMENT "Generating API documentation with Doxygen" VERBATIM)
add_custom_target(msl_doc ALL DEPENDS ${outputFile})

msl_set_folder(msl_doc "")

if (MSL_INSTALL)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()
