if (MSL_BUILD_FLATBUFFERS)
	# Touch the .fbs file on generation to avoid situations where the timestamp doesn't rebuild the
	# generated header checked into the repository but the system version of flatbuffers is
	# different and incompatible. Don't use VERSION_GREATER_EQUAL to support older CMake versions.
	set(flatbufferFile ${CMAKE_CURRENT_SOURCE_DIR}/mslb.fbs)
	if (CMAKE_VERSION VERSION_GREATER 3.11.99)
		file(TOUCH_NOCREATE ${flatbufferFile})
	elseif (NOT WIN32)
		execute_process(COMMAND touch -c ${flatbufferFile})
	endif()

	add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/mslb_generated.h
		MAIN_DEPENDENCY ${flatbufferFile}
		COMMAND ${FLATC} ARGS -c --scoped-enums --no-includes --gen-mutable --no-warnings mslb.fbs
		DEPENDS ${FLATC}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	add_custom_target(mslb DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mslb_generated.h)
else()
	add_custom_target(mslb)
endif()

msl_set_folder(mslb libs)

if (MSL_INSTALL)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include COMPONENT dev)
endif()
