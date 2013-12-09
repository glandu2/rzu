cmake_minimum_required(VERSION 2.8)

file(GLOB SOURCE_FILES *.cpp *.h)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} RappelzLib)
target_link_libraries(${CMAKE_PROJECT_NAME} crypto uv)
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ./)
