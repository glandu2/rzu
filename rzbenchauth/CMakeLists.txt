cmake_minimum_required(VERSION 2.8)
project(RappelzPlayerCount)

set(TARGET_NAME RappelzPlayerCount)

include_directories("${CMAKE_CURRENT_LIST_DIR}/../RappelzLib/src")
link_directories("${CMAKE_CURRENT_LIST_DIR}/../RappelzLib/build-linux-amd64-bin/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/../libuv/include")
link_directories("${CMAKE_CURRENT_LIST_DIR}/../libuv/build-linux-amd64-bin")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src})
