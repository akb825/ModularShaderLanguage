find_program(FLATC flatc)

if (FLATC)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/mslb_generated.h
		MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/mslb.fbs
		COMMAND ${FLATC} ARGS -c --scoped-enums --no-includes --gen-mutable mslb.fbs
		DEPENDS ${FLATC}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	add_custom_target(mslb DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mslb_generated.h)
else()
	message("flatc not installed. Using pre-compiled flatbuffer.")
	add_custom_target(mslb)
endif()

msl_set_folder(mslb libs)

if (MSL_INSTALL)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include COMPONENT dev)
endif()
