if(__add_version_info_cmake)
	return()
endif()
set(__add_version_info_cmake YES)

function(add_git_version_info _targetname _sourcefiles)
	include_directories(${CMAKE_BINARY_DIR})
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp"
		COMMAND ${CMAKE_COMMAND} -D SRC=${CUSTOM_CMAKE_MODULES_PATH}/GitVersion.cpp.in
			-D DST=${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp
			-D TARGET_NAME=${TARGET_NAME}
			-D WORKINGDIR=${CMAKE_CURRENT_LIST_DIR}
			-P ${CUSTOM_CMAKE_MODULES_PATH}/GetGitVersion.cmake
		DEPENDS
			${${_sourcefiles}}
			${CUSTOM_CMAKE_MODULES_PATH}/GetGitVersion.cmake
			${CUSTOM_CMAKE_MODULES_PATH}/GitVersion.cpp.in
	)
	configure_file(${CUSTOM_CMAKE_MODULES_PATH}/GitVersion.h.in ${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h @ONLY)
	set(${_sourcefiles} ${${_sourcefiles}} "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp" "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h" PARENT_SCOPE)
endfunction()
