cmake_minimum_required(VERSION 2.8.12)
project(rzauctionmonitor)

set(TARGET_NAME rzauctionmonitor)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib)

add_subdirectory(parser)
