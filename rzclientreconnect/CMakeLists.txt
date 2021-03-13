cmake_minimum_required(VERSION 2.8.12)
project(rzclientreconnect)

set(TARGET_NAME rzclientreconnect)

add_subdirectory(src)

install(DIRECTORY dist/ DESTINATION ./)
install(FILES README.md DESTINATION ./ RENAME ${TARGET_NAME}.txt)
