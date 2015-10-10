cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

include(AddVersionInfo OPTIONAL RESULT_VARIABLE versioncmakefound)
if( versioncmakefound )
    add_git_version_info(${TARGET_NAME}_GitVersion SOURCE_FILES)
endif()

add_exe(${TARGET_NAME} "${SOURCE_FILES}" rzu)

if(CMAKE_COMPILER_IS_GNUCXX)
  SETUP_TARGET_FOR_COVERAGE(${TARGET_NAME}_lcov "${TARGET_NAME}" "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}_coverage")
endif()
