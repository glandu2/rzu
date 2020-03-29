cmake_minimum_required(VERSION 2.8.12)
project(rzfilter)

set(TARGET_NAME rzfilter)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(filter)
add_subdirectory(test)

install(DIRECTORY dist/ DESTINATION ./)
install(DIRECTORY include/ DESTINATION ./include)
