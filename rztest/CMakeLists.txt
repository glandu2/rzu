cmake_minimum_required(VERSION 2.6)
project(rztest)

set(TARGET_NAME rztest)

if(BUILD_SHARED_LIBS)
  add_definitions(-DUSING_UV_SHARED)
  add_definitions(-DUSING_RZU_SHARED)
endif()
include_directories("${RZU_INCLUDE}")
include_directories("${UV_INCLUDE}")
set(RZTEST_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src" CACHE STRING "rztest include directory")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src)
