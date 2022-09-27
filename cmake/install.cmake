# Copyright 2018-2022 Aaron Barany
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
	set(namespacedTarget MSL::${ARGS_MODULE})
	string(TOUPPER ${ARGS_MODULE} moduleUpper)

	set_target_properties(${ARGS_TARGET} PROPERTIES
		VERSION ${MSL_VERSION}
		DEBUG_POSTFIX d
		EXPORT_NAME ${ARGS_MODULE})
	add_library(${namespacedTarget} ALIAS ${ARGS_TARGET})

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
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		COMPONENT dev)
	install(FILES ${exportPath} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MSL/${ARGS_MODULE}
		COMPONENT dev)

	include(CMakePackageConfigHelpers)
	set(versionPath ${MSL_EXPORTS_DIR}/${moduleName}ConfigVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${MSL_VERSION}
		COMPATIBILITY SameMajorVersion)

	export(EXPORT ${moduleName}Targets
		FILE ${MSL_EXPORTS_DIR}/${moduleName}Targets.cmake)

	if (ARGS_DEPENDS OR ARGS_EXTERNAL_DEPENDS OR ARGS_BOOST_DEPENDS)
		set(dependencies "include(CMakeFindDependencyMacro)")
	else()
		set(dependencies)
	endif()
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
		"set(MSL${ARGS_MODULE}_LIBRARIES ${namespacedTarget})\n"
		"get_target_property(MSL${ARGS_MODULE}_INCLUDE_DIRS ${namespacedTarget} INTERFACE_INCLUDE_DIRECTORIES)\n")

	set(configPackageDir ${CMAKE_INSTALL_LIBDIR}/cmake/${moduleName})
	install(EXPORT ${moduleName}Targets NAMESPACE MSL:: FILE ${moduleName}Targets.cmake
		DESTINATION ${configPackageDir})
	install(FILES ${configPath} ${versionPath} DESTINATION ${configPackageDir} COMPONENT dev)
endfunction()

function(msl_install_executable target)
	add_executable(MSL::${name} ALIAS ${target})

	if (NOT MSL_INSTALL)
		return()
	endif()

	install(TARGETS ${target} EXPORT ${target}Targets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	export(EXPORT ${target}Targets FILE ${MSL_EXPORTS_DIR}/${target}-targets.cmake)
	install(EXPORT ${target}Targets NAMESPACE MSL:: FILE ${target}-targets.cmake
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MSL)
	set_property(GLOBAL APPEND PROPERTY MSL_TOOL_TARGETS ${target}-targets.cmake)
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

	get_property(toolTargets GLOBAL PROPERTY MSL_TOOL_TARGETS)
	set(toolTargetLines)
	foreach (toolTarget ${toolTargets})
		set(toolTargetLines "${toolTargetLines}\ninclude(\${CMAKE_CURRENT_LIST_DIR}/${toolTarget})")
	endforeach()

	set(configPackageDir ${CMAKE_INSTALL_LIBDIR}/cmake/MSL)
	configure_file(${MSL_SOURCE_DIR}/cmake/templates/MSLConfig.cmake.in
		${MSL_EXPORTS_DIR}/MSLConfig.cmake @ONLY)
	install(FILES ${MSL_EXPORTS_DIR}/MSLConfig.cmake ${versionPath} DESTINATION ${configPackageDir}
		COMPONENT dev)
endfunction()
