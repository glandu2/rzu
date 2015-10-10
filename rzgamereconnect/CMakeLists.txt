cmake_minimum_required(VERSION 2.8.12)
project(rzgamereconnect)

set(TARGET_NAME rzgamereconnect)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" src)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test" test)
