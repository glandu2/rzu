if(__add_version_info_cmake)
        return()
endif()
set(__add_version_info_cmake YES)

set(GetGitVersion_LOCATION "${CMAKE_CURRENT_LIST_DIR}")

function(add_git_version_info _targetname _sourcefiles)
	include_directories(${CMAKE_BINARY_DIR})
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp"
		COMMAND ${CMAKE_COMMAND} -D SRC=${GetGitVersion_LOCATION}/GitVersion.cpp.in
			-D DST=${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp
			-D TARGET_NAME=${TARGET_NAME}
			-D WORKINGDIR=${CMAKE_CURRENT_LIST_DIR}
			-P ${GetGitVersion_LOCATION}/GetGitVersion.cmake
		DEPENDS ${${_sourcefiles}} ${GetGitVersion_LOCATION}/GetGitVersion.cmake
	)
	configure_file(${GetGitVersion_LOCATION}/GitVersion.h.in ${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h @ONLY)
	set(${_sourcefiles} ${${_sourcefiles}} "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp" "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h" PARENT_SCOPE)
endfunction()
