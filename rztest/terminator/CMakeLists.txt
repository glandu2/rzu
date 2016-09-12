cmake_minimum_required(VERSION 2.8.12)

file(GLOB SOURCE_FILES *.cpp *.h)

include(AddVersionInfo OPTIONAL RESULT_VARIABLE versioncmakefound)
if( versioncmakefound )
	add_git_version_info(${TARGET_NAME}_GitVersion SOURCE_FILES)
endif()

add_exe(rzterminator "${SOURCE_FILES}" rzu rztest)
