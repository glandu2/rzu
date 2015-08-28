cmake_minimum_required(VERSION 2.8)
project(rzlog)

set(TARGET_NAME rzlog)

if(BUILD_SHARED_LIBS)
  add_definitions(-DUSING_UV_SHARED)
  add_definitions(-DUSING_RZU_SHARED)
endif()
include_directories(${RZU_INCLUDE})
include_directories(${UV_INCLUDE})

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src)
#add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test" test)
