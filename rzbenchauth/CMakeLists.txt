cmake_minimum_required(VERSION 2.8)
project(RappelzBenchmark)

set(TARGET_NAME RappelzBenchmark)

if(BUILD_SHARED_LIBS)
  add_definitions(-DUSING_UV_SHARED)
  add_definitions(-DUSING_RAPPELZLIB)
endif()
include_directories("${CMAKE_CURRENT_LIST_DIR}/../RappelzLib/src")
include_directories("${CMAKE_CURRENT_LIST_DIR}/../libuv/include")

find_package(OpenSSL REQUIRED)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src)
