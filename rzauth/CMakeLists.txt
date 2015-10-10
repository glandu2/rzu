cmake_minimum_required(VERSION 2.8.12)
project(rzauth)

set(TARGET_NAME rzauth)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test" test)
