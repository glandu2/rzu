cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

include(AddVersionInfo OPTIONAL RESULT_VARIABLE versioncmakefound)
if( versioncmakefound )
    add_git_version_info(${TARGET_NAME}_GitVersion SOURCE_FILES)
endif()

add_library(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME}  rzu gtest)
target_include_directories(${TARGET_NAME} PUBLIC ".")

if(BUILD_SHARED_LIBS)
  target_compile_definitions(${TARGET_NAME}
    PRIVATE BUILDING_LIBRZTEST
    INTERFACE USING_RZTEST_SHARED
  )
endif()
