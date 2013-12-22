cmake_minimum_required(VERSION 2.8)
project(RappelzPlayerCount)

set(TARGET_NAME RappelzPlayerCount)

add_definitions(-DUSING_UV_SHARED)
add_definitions(-DUSING_RAPPELZLIB)
include_directories("${CMAKE_CURRENT_LIST_DIR}/../RappelzLib/src")
include_directories("${CMAKE_CURRENT_LIST_DIR}/../libuv/include")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src})
