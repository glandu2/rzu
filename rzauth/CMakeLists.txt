cmake_minimum_required(VERSION 2.8)
project(RappelzServerAuth)

set(TARGET_NAME RappelzServerAuth)

include_directories("${CMAKE_CURRENT_LIST_DIR}/../RappelzLib/src")
link_directories("${CMAKE_CURRENT_LIST_DIR}/../RappelzLib/build-linux-amd64-bin/")
include_directories("${CMAKE_CURRENT_LIST_DIR}/../libuv/include")
link_directories("${CMAKE_CURRENT_LIST_DIR}/../libuv/build-linux-amd64-bin")

find_package(OpenSSL REQUIRED)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src})
