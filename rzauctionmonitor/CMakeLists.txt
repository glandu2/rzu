cmake_minimum_required(VERSION 2.8.12)
project(rzauctionmonitor)

set(TARGET_NAME rzauctionmonitor)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test" test)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/lib" lib)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/parser" parser)
