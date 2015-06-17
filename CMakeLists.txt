cmake_minimum_required(VERSION 2.8)
project(rzu-parent)
enable_testing()
cmake_policy(SET CMP0042 NEW) # MACOSX_RPATH = ON by default

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

set(CMAKE_VERBOSE_MAKEFILE false CACHE BOOL "Print compilation commands")
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (one of Debug, Release, RelWithDebInfo, MinSizeRel)")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs instead of static")

message("Using ${CMAKE_BUILD_TYPE} mode")
message("Using shared libs: ${BUILD_SHARED_LIBS}")

get_filename_component(BUILD_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/../${BUILD_DIR_NAME}-bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/../${BUILD_DIR_NAME}-bin")

include(FindCXXFeatures)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO /INCREMENTAL:NO")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO /INCREMENTAL:NO")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO /INCREMENTAL:NO")

  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -Wall")

  if(NOT ${CMAKE_SYSTEM_NAME} MATCHES AIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  endif()
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
  endif()


  if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti  -static-libgcc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions  -static-libgcc")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
  endif()
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/")

include(AddTestableExecutable)
if(CMAKE_COMPILER_IS_GNUCXX)
  include(CodeCoverage)
endif()

# Compile subprojects if existing
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/libuv/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/libuv" libuv)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/zlib/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/zlib" zlib)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/gtest/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/gtest" gtest)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/librzu/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/librzu" librzu)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rztest/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rztest" rztest)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzauth/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzauth" rzauth)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzgame/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzgame" rzgame)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzplayercount/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzplayercount" rzplayercount)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzbenchauth/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzbenchauth" rzbenchauth)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzchatgateway/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzchatgateway" rzchatgateway)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzgamereconnect/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzgamereconnect" rzgamereconnect)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzfilter/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzfilter" rzfilter)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/VStructGen/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/VStructGen" VStructGen)
endif()

# Packaging

SET(CPACK_NSIS_DISPLAY_NAME "Rappelz Emu - Glandu2")
SET(CPACK_PACKAGE_NAME "rzemu")
SET(CPACK_PACKAGE_VENDOR "Glandu2")
SET(CPACK_PACKAGE_VERSION "4.0")
SET(CPACK_PACKAGE_VERSION_MAJOR "4")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_OUTPUT_FILE_PREFIX ${EXECUTABLE_OUTPUT_PATH})
SET(CPACK_STRIP_FILES ON)
SET(CMAKE_INSTALL_PREFIX "")
include(cpack OPTIONAL)
