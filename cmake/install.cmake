function(install_library)
	set(options)
	set(oneValueArgs TARGET MODULE)
	set(multiValueArgs DEPENDENCIES)
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
			file(WRITE ${exportPath}
				"#pragma once\n\n"
				"#ifdef MSL_${moduleUpper}_BUILD\n"
				"#define MSL_${moduleUpper}_EXPORT __declspec(dllexport)\n"
				"#else\n"
				"#define MSL_${moduleUpper}_EXPORT __declspec(dllimport)\n"
				"#endif\n")
		elseif (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
			file(WRITE ${exportPath}
				"#pragma once\n\n"
				"#define MSL_${moduleUpper}_EXPORT __attribute__((visibility(\"default\")))\n")
		else()
			file(WRITE ${exportPath}
				"#pragma once\n\n"
				"#define MSL_${moduleUpper}_EXPORT\n")
		endif()
	else()
		file(WRITE ${exportPath}
			"#pragma once\n\n"
			"#define MSL_${moduleUpper}_EXPORT\n")
	endif()

	install(TARGETS ${ARGS_TARGET} EXPORT ${moduleName}Targets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include COMPONENT dev)
	install(FILES ${exportPath} DESTINATION include/MSL/${ARGS_MODULE} COMPONENT dev)

	include(CMakePackageConfigHelpers)
	set(versionPath ${MSL_EXPORTS_DIR}/${moduleName}Version.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${MSL_VERSION}
		COMPATIBILITY AnyNewerVersion)

	export(EXPORT ${moduleName}Targets
		FILE ${MSL_EXPORTS_DIR}/${moduleName}Targets.cmake)

	set(dependencies "include(CMakeFindDependencyMacro)")
	foreach (dependency ${ARGS_DEPENDENCIES})
		set(dependencies "${dependencies}\nfind_dependency(MSL${dependency} ${MSL_VERSION})")
	endforeach()

	set(configPath ${MSL_EXPORTS_DIR}/${moduleName}Config.cmake)
	file(WRITE ${configPath}
		"${dependencies}\n"
		"include(\${CMAKE_CURRENT_LIST_DIR}/${moduleName}Targets.cmake\n"
		"get_target_property(MSL${ARGS_MODULE}_LIBRARIES ${ARGS_TARGET} INTERFACE_LINK_LIBRARIES)\n"
		"get_target_property(MSL${ARGS_MODULE}_INCLUDE_DIRS ${ARGS_TARGET} INTERFACE_INCLUDE_DIRECTORIES)\n")

	set(configPackageDir lib/cmake/MSL)
	install(EXPORT ${moduleName}Targets FILE ${moduleName}Targets.cmake
		DESTINATION ${configPackageDir})
	install(FILES ${configPath} ${versionPath} DESTINATION ${configPackageDir} COMPONENT dev)
endfunction()

function(install_master_config)
	include(CMakePackageConfigHelpers)
	set(versionPath ${MSL_EXPORTS_DIR}/MSLVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${MSL_VERSION}
		COMPATIBILITY AnyNewerVersion)

	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MSLConfig.cmake
		DESTINATION ${MSL_EXPORTS_DIR})
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MSLConfig.cmake ${versionPath}
		DESTINATION lib/cmake/MSL COMPONENT dev)
endfunction()
