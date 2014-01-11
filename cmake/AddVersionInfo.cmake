if(__add_version_info_cmake)
        return()
endif()
set(__add_version_info_cmake YES)

set(GetGitVersion_LOCATION "${CMAKE_CURRENT_LIST_DIR}")

function(add_git_version_info _targetname _sourcefiles)
	include_directories(${CMAKE_BINARY_DIR})
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp" "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h"
		COMMAND ${CMAKE_COMMAND} -D SRC=${GetGitVersion_LOCATION}/GitVersion
			-D DST=${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion
			-D TARGET_NAME=${TARGET_NAME}
			-D WORKINGDIR=${CMAKE_CURRENT_LIST_DIR}
			-P ${GetGitVersion_LOCATION}/GetGitVersion.cmake
		DEPENDS ${${_sourcefiles}}
	)
	set(${_sourcefiles} ${${_sourcefiles}} "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp" "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h" PARENT_SCOPE)
endfunction()


#function(add_git_version_info _targetname _sourcefiles)
#	include_directories(${CMAKE_BINARY_DIR})
#	add_custom_target(${_targetname}
#		COMMAND ${CMAKE_COMMAND} -D SRC=${GetGitVersion_LOCATION}/GitVersion
#			-D DST=${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion
#			-D TARGET_NAME=${TARGET_NAME}
#			-P ${GetGitVersion_LOCATION}/GetGitVersion.cmake
#	)
#	set_source_files_properties("${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp" "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h" PROPERTIES GENERATED TRUE)
#	set(${_sourcefiles} ${${_sourcefiles}} "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.cpp" "${CMAKE_BINARY_DIR}/${TARGET_NAME}GitVersion.h" PARENT_SCOPE)
#endfunction()
