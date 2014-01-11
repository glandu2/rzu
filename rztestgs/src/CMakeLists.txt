cmake_minimum_required(VERSION 2.8)

file(GLOB SOURCE_FILES *.cpp *.h)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/AddVersionInfo.cmake)
add_git_version_info(${TARGET_NAME}_GitVersion SOURCE_FILES)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
add_dependencies(${TARGET_NAME} ${TARGET_NAME}_GitVersion)
target_link_libraries(${TARGET_NAME} RappelzLib uv)
install(TARGETS ${TARGET_NAME} DESTINATION ./)
