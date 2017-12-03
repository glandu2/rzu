cmake_minimum_required(VERSION 2.8.12)
project(rzu-parent)
enable_testing()

set(CUSTOM_CMAKE_MODULES_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${CUSTOM_CMAKE_MODULES_PATH}")

set(CMAKE_VERBOSE_MAKEFILE false CACHE BOOL "Print compilation commands")
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (one of Debug, Release, RelWithDebInfo, MinSizeRel)")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs instead of static")

# Packaging

set(CPACK_NSIS_DISPLAY_NAME "Rappelz Emu - Glandu2")
set(CPACK_PACKAGE_NAME "rzemu")
set(CPACK_PACKAGE_VENDOR "Glandu2")
set(CPACK_PACKAGE_VERSION "4.1" CACHE STRING "Version string")
set(CPACK_GENERATOR "ZIP")
set(CPACK_STRIP_FILES ON)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
message("Package name: ${CPACK_PACKAGE_FILE_NAME}")

message("Using ${CMAKE_BUILD_TYPE} mode")
message("Using shared libs: ${BUILD_SHARED_LIBS}")

get_filename_component(BUILD_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${BUILD_DIR_NAME}-bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${BUILD_DIR_NAME}-bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${BUILD_DIR_NAME}-bin")

set(CMAKE_CXX_STANDARD 11)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy" "clang-tidy-6.0" "clang-tidy-5.0"
  DOC "Path to clang-tidy executable"
  )
if(CLANG_TIDY_EXE)
	option(USE_CLANG_TIDY "Enable use of clang-tidy")
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
  if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
  endif()
  add_compile_options(/wd4251 /wd4275)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -Wall")

  if(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
	if(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
	  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
	  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
	  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
	endif()
  endif()

  if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
  endif()

  add_definitions(-D__STDC_FORMAT_MACROS)
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")

  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_INSTALL_RPATH "@loader_path/")
else()
  set(CMAKE_INSTALL_RPATH "$ORIGIN/")
endif()

include(AddTestableExecutable)
if(CMAKE_COMPILER_IS_GNUCXX)
  include(CodeCoverage)
endif()



# Compile subprojects if existing
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "deps")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/libuv/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/libuv" libuv)
  install_pdb("uv")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/libiconv/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/libiconv" libiconv)
  install_pdb("iconv")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/zlib/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/zlib" zlib)
  install_pdb("z")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/gtest/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/gtest" gtest)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/liblua/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/liblua" liblua)
endif()


if(CLANG_TIDY_EXE AND USE_CLANG_TIDY)
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix-errors" "-checks=*,-google-readability-braces-around-statements,-hicpp-braces-around-statements,-readability-braces-around-statements")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/librzu/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/librzu" librzu)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rztest/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rztest" rztest)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzauth")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzauth/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzauth" rzauth)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzgame")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzgame/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzgame" rzgame)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzplayercount")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzplayercount/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzplayercount" rzplayercount)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzbenchauth")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzbenchauth/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzbenchauth" rzbenchauth)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzchatgateway")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzchatgateway/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzchatgateway" rzchatgateway)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzgamereconnect")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzgamereconnect/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzgamereconnect" rzgamereconnect)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzlog")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzlog/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzlog" rzlog)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzbenchlog")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzbenchlog/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzbenchlog" rzbenchlog)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzfilter")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzfilter/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzfilter" rzfilter)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzauctionmonitor")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzauctionmonitor/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rzauctionmonitor" rzauctionmonitor)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rztestgs/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/rztestgs" rztestgs)
endif()

include(CPack OPTIONAL)
