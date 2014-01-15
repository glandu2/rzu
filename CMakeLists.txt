cmake_minimum_required(VERSION 2.8)
project(RappelzUnified)

set(CMAKE_VERBOSE_MAKEFILE false CACHE BOOL "Print compilation commands")
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (one of Debug, Release, RelWithDebInfo, MinSizeRel)")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs instead of static")

message("Using ${CMAKE_BUILD_TYPE} mode")

get_filename_component(BUILD_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/../${BUILD_DIR_NAME}-bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/../${BUILD_DIR_NAME}-bin")

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO /INCREMENTAL:NO")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO /INCREMENTAL:NO")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO /INCREMENTAL:NO")

  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -Wall -fvisibility=hidden")

  if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
  endif()

  #for qtcreator
  add_definitions("-D__GXX_EXPERIMENTAL_CXX0X__")
endif()

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,.")
  set(CMAKE_SKIP_RPATH TRUE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

# Compile subprojects if existing
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/libuv/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/libuv" libuv)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/RappelzLib/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/RappelzLib" RappelzLib)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/RappelzServerAuth/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/RappelzServerAuth" RappelzServerAuth)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/RappelzPlayerCount/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/RappelzPlayerCount" RappelzPlayerCount)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/RappelzAuthBenchmark/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/RappelzAuthBenchmark" RappelzAuthBenchmark)
endif()
