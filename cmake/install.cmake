# Copyright 2018-2020 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

function(msl_install_library)
	set(options)
	set(oneValueArgs TARGET MODULE)
	set(multiValueArgs DEPENDS EXTERNAL_DEPENDS BOOST_DEPENDS)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(moduleName MSL${ARGS_MODULE})
	string(TOUPPER ${ARGS_MODULE} moduleUpper)

	set_property(TARGET ${ARGS_TARGET} PROPERTY VERSION ${MSL_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY SOVERSION ${MSL_MAJOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_MAJOR_VERSION
		${MSL_MAJOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_MINOR_VERSION
		${MSL_MINOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_PATCH_VERSION
		${MSL_PATCH_VERSION})
	set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY COMPATIBLE_VERSION_STRING
		${moduleName}_MAJOR_VERSION)
	set_property(TARGET ${ARGS_TARGET} PROPERTY DEBUG_POSTFIX d)

	set(interfaceIncludes
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<BUILD_INTERFACE:${SHARED_DIR}/include>)
	set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
		${interfaceIncludes})

	set(exportPath ${CMAKE_CURRENT_BINARY_DIR}/include/MSL/${ARGS_MODULE}/Export.h)
	set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY INCLUDE_DIRECTORIES
		${CMAKE_CURRENT_BINARY_DIR}/include ${interfaceIncludes})
	if (MSL_SHARED)
		if (MSVC)
			set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS
				MSL_${moduleUpper}_BUILD)
			configure_file(${MSL_SOURCE_DIR}/cmake/templates/WindowsExport.h.in ${exportPath} @ONLY)
		else()
			configure_file(${MSL_SOURCE_DIR}/cmake/templates/UnixExport.h.in ${exportPath} @ONLY)
		endif()
	else()
		configure_file(${MSL_SOURCE_DIR}/cmake/templates/NoExport.h.in ${exportPath} @ONLY)
	endif()

	if (NOT MSL_INSTALL)
		return()
	endif()

	install(TARGETS ${ARGS_TARGET} EXPORT ${moduleName}Targets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include COMPONENT dev)
	install(FILES ${exportPath} DESTINATION include/MSL/${ARGS_MODULE} COMPONENT dev)

	include(CMakePackageConfigHelpers)
	set(versionPath ${MSL_EXPORTS_DIR}/${moduleName}ConfigVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${MSL_VERSION}
		COMPATIBILITY SameMajorVersion)

	export(EXPORT ${moduleName}Targets
		FILE ${MSL_EXPORTS_DIR}/${moduleName}Targets.cmake)

	set(dependencies "include(CMakeFindDependencyMacro)")
	foreach (dependency ${ARGS_DEPENDS})
		set(dependencies "${dependencies}\nfind_dependency(MSL${dependency} ${MSL_VERSION} EXACT)")
	endforeach()
	foreach (dependency ${ARGS_EXTERNAL_DEPENDS})
		set(dependencies "${dependencies}\nfind_dependency(${dependency})")
	endforeach()
	if (ARGS_BOOST_DEPENDS)
		string(REPLACE ";" " " boostDepends "${ARGS_BOOST_DEPENDS}")
		set(dependencies "${dependencies}\nfind_dependency(Boost COMPONENTS ${boostDepends})")
	endif()

	set(configPath ${MSL_EXPORTS_DIR}/${moduleName}Config.cmake)
	file(WRITE ${configPath}
		"${dependencies}\n"
		"include(\${CMAKE_CURRENT_LIST_DIR}/${moduleName}Targets.cmake)\n"
		"set(MSL${ARGS_MODULE}_LIBRARIES ${ARGS_TARGET})\n"
		"get_target_property(MSL${ARGS_MODULE}_INCLUDE_DIRS ${ARGS_TARGET} INTERFACE_INCLUDE_DIRECTORIES)\n")

	if (WIN32)
		set(configPackageDir ${moduleName}/cmake)
	else()
		set(configPackageDir lib/cmake/${moduleName})
	endif()
	install(EXPORT ${moduleName}Targets FILE ${moduleName}Targets.cmake
		DESTINATION ${configPackageDir})
	install(FILES ${configPath} ${versionPath} DESTINATION ${configPackageDir} COMPONENT dev)
endfunction()

function(msl_install_master_config)
	if (NOT MSL_INSTALL)
		return()
	endif()

	include(CMakePackageConfigHelpers)
	set(versionPath ${MSL_EXPORTS_DIR}/MSLConfigVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${MSL_VERSION}
		COMPATIBILITY SameMajorVersion)

	if (WIN32)
		set(configPackageDir MSL/cmake)
	else()
		set(configPackageDir lib/cmake/MSL)
	endif()
	file(COPY ${MSL_SOURCE_DIR}/cmake/templates/MSLConfig.cmake DESTINATION ${MSL_EXPORTS_DIR})
	install(FILES ${MSL_SOURCE_DIR}/cmake/templates/MSLConfig.cmake ${versionPath}
		DESTINATION ${configPackageDir} COMPONENT dev)
endfunction()
