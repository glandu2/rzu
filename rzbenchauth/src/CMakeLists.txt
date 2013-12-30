cmake_minimum_required(VERSION 2.8)

file(GLOB SOURCE_FILES *.cpp *.h)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} RappelzLib uv ${OPENSSL_LIBRARIES})
install(TARGETS ${TARGET_NAME} DESTINATION ./)
