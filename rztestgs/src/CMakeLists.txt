cmake_minimum_required(VERSION 2.8)

file(GLOB SOURCE_FILES *.cpp *.h)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} RappelzLib uv)
install(TARGETS ${TARGET_NAME} DESTINATION ./)
