if (MSL_BUILD_COMPILE)
	add_subdirectory(mslc)
endif()

if (MSL_BUILD_CLIENT)
	add_subdirectory(mslb-extract)
endif()
