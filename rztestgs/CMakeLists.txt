cmake_minimum_required(VERSION 2.8)
project(PlayerCount)

set(TARGET_NAME PlayerCount)

if(BUILD_SHARED_LIBS)
  add_definitions(-DUSING_UV_SHARED)
  add_definitions(-DUSING_RZU_SHARED)
endif()
include_directories("${CMAKE_CURRENT_LIST_DIR}/../librzu/src")
include_directories("${CMAKE_CURRENT_LIST_DIR}/../libuv/include")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src)
