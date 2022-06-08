cmake_minimum_required(VERSION 3.1)
project(rzu-parent)
cmake_policy(SET CMP0065 NEW)

enable_testing()

set(CUSTOM_CMAKE_MODULES_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${CUSTOM_CMAKE_MODULES_PATH}")

set(CMAKE_VERBOSE_MAKEFILE false CACHE BOOL "Print compilation commands")
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (one of Debug, Release, RelWithDebInfo, MinSizeRel)")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs instead of static")


# Packaging
find_package(Git QUIET)
if(GIT_FOUND)
execute_process(
	COMMAND ${GIT_EXECUTABLE} describe --tags --always
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
else()
set(GIT_VERSION "5.7")
endif()


set(CPACK_NSIS_DISPLAY_NAME "rzu - Glandu2")
set(CPACK_PACKAGE_NAME "rzemu")
set(CPACK_PACKAGE_VENDOR "Glandu2")
if(NOT DEFINED CPACK_PACKAGE_VERSION)
	set(CPACK_PACKAGE_VERSION "${GIT_VERSION}")
endif()
set(CPACK_GENERATOR "ZIP")
set(CPACK_STRIP_FILES ON)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
message("Package name: ${CPACK_PACKAGE_FILE_NAME}")

message("Using ${CMAKE_BUILD_TYPE} mode")
message("Using shared libs: ${BUILD_SHARED_LIBS}")

if(NOT DEFINED RZU_BUILD_DIR_NAME)
	get_filename_component(RZU_DEFAULT_BUILD_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
	set(RZU_BUILD_DIR_NAME "${CMAKE_BINARY_DIR}/../${RZU_DEFAULT_BUILD_DIR_NAME}-bin" CACHE PATH "Path where to put compiled binaries")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${RZU_BUILD_DIR_NAME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${RZU_BUILD_DIR_NAME}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${RZU_BUILD_DIR_NAME}")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${RZU_BUILD_DIR_NAME}")

set(CMAKE_CXX_STANDARD 17)

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
  add_compile_options(/wd4251 /wd4275 /bigobj)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -Wall")

  if(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")
	set(ALL_COMPILER_FLAGS "-fvisibility=hidden")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALL_COMPILER_FLAGS} -fvisibility-inlines-hidden")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALL_COMPILER_FLAGS}")
	if(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
	  set(ALL_LINKER_FLAGS "-Wl,--as-needed")
	  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ALL_LINKER_FLAGS}")
	  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${ALL_LINKER_FLAGS}")
	  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ALL_LINKER_FLAGS}")
	endif()
  endif()

  if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -Wa,-mbig-obj")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -Wa,-mbig-obj")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias -static-libgcc")
  endif()

endif()
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-D__STDC_LIMIT_MACROS)

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

include(FetchContent)
FetchContent_Declare(
  curl
  URL     https://github.com/curl/curl/releases/download/curl-7_72_0/curl-7.72.0.tar.xz
)

set(HTTP_ONLY ON)
set(CMAKE_USE_OPENSSL ON)
set(BUILD_CURL_EXE OFF)
set(BUILD_TESTING OFF)
FetchContent_MakeAvailable(curl)
install(TARGETS libcurl
  RUNTIME DESTINATION ./
  LIBRARY DESTINATION ./
)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/libuv/CMakeLists.txt")
  add_subdirectory(libuv)
  install_pdb("uv")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/libiconv/CMakeLists.txt")
  add_subdirectory(libiconv)
  install_pdb("iconv")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/zlib/CMakeLists.txt")
  add_subdirectory(zlib)
  install_pdb("z")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/gtest/CMakeLists.txt")
  set(BUILD_GMOCK OFF CACHE BOOL "")
  set(INSTALL_GTEST OFF CACHE BOOL "")
  set(gtest_hide_internal_symbols ON CACHE BOOL "")
  add_subdirectory(gtest EXCLUDE_FROM_ALL)
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(gtest
      PRIVATE GTEST_CREATE_SHARED_LIBRARY=1
      INTERFACE GTEST_LINKED_AS_SHARED_LIBRARY=1
    )
  endif()
  set_target_properties(gtest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
    PDB_OUTPUT_DIRECTORY "${CMAKE_PDB_OUTPUT_DIRECTORY}")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/liblua/CMakeLists.txt")
  add_subdirectory(liblua)
endif()


if(CLANG_TIDY_EXE AND USE_CLANG_TIDY)
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix-errors" "-checks=*,-google-readability-braces-around-statements,-hicpp-braces-around-statements,-readability-braces-around-statements")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/librzu/CMakeLists.txt")
  add_subdirectory(librzu)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rztest/CMakeLists.txt")
  add_subdirectory(rztest)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzauth")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzauth/CMakeLists.txt")
  add_subdirectory(rzauth)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzgame")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzgame/CMakeLists.txt")
  add_subdirectory(rzgame)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzplayercount")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzplayercount/CMakeLists.txt")
  add_subdirectory(rzplayercount)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzbenchauth")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzbenchauth/CMakeLists.txt")
  add_subdirectory(rzbenchauth)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzchatgateway")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzchatgateway/CMakeLists.txt")
  add_subdirectory(rzchatgateway)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzgamereconnect")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzgamereconnect/CMakeLists.txt")
  add_subdirectory(rzgamereconnect)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzclientreconnect")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzclientreconnect/CMakeLists.txt")
  add_subdirectory(rzclientreconnect)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzlog")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzlog/CMakeLists.txt")
  add_subdirectory(rzlog)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzbenchlog")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzbenchlog/CMakeLists.txt")
  add_subdirectory(rzbenchlog)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzfilter")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzfilter/CMakeLists.txt")
  add_subdirectory(rzfilter)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzauctionmonitor")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzauctionmonitor/CMakeLists.txt")
  add_subdirectory(rzauctionmonitor)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rztestgs")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rztestgs/CMakeLists.txt")
  add_subdirectory(rztestgs)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "rzrequester")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/rzrequester/CMakeLists.txt")
  add_subdirectory(rzrequester)
endif()

include(CPack OPTIONAL)

# VSCode support
add_custom_target(do_package DEPENDS package)
configure_file(cmake/clangd.in ${CMAKE_SOURCE_DIR}/.clangd @ONLY)
