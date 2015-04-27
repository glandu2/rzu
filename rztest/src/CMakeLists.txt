cmake_minimum_required(VERSION 2.6)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

include(AddVersionInfo OPTIONAL RESULT_VARIABLE versioncmakefound)
if( versioncmakefound )
    add_git_version_info(${TARGET_NAME}_GitVersion SOURCE_FILES)
endif()

set(LIBS rzu gtest)
add_lib(${TARGET_NAME} "${SOURCE_FILES}" "${LIBS}")
