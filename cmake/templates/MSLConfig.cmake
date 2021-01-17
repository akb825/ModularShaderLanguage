if (CMAKE_VERSION VERSION_LESS 3.0.2)
	message(FATAL_ERROR "MSL requires at least CMake version 3.0.2")
endif()

if (NOT MSL_FIND_COMPONENTS)
	set(MSL_NOT_FOUND_MESSAGE "The MSL package requires at least one component")
	set(MSL_FOUND False)
	return()
endif()

get_filename_component(_MSL_InstallPrefix ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)

include(CMakeFindDependencyMacro)
set(_MSL_NOTFOUND_MESSAGE)
foreach(module ${MSL_FIND_COMPONENTS})
	find_dependency(MSL${module})
	if (NOT MSL${module}_FOUND)
		if (MSL_FIND_REQUIRED_${module})
			set(_MSL_NOTFOUND_MESSAGE "${_MSL_NOTFOUND_MESSAGE}Failed to find MSL component \"${module}\" config file at \"${_MSL_InstallPrefix}/MSL${module}/MSL${module}Config.cmake\"\n")
		elseif(NOT MSL_FIND_QUIETLY)
			message(WARNING "Failed to find MSL component \"${module}\" config file at \"${_MSL_InstallPrefix}/MSL${module}/MSL${module}Config.cmake\"")
		endif()
	endif()
endforeach()

if (_MSL_NOTFOUND_MESSAGE)
	set(MSL_NOT_FOUND_MESSAGE "${_MSL_NOTFOUND_MESSAGE}")
	set(MSL_FOUND False)
endif()