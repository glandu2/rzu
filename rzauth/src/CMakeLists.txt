cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} RappelzLib uv)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} ${OPENSSL_LIBRARIES})
if(WIN32)
    target_link_libraries(${TARGET_NAME} odbc32)
else()
    target_link_libraries(${TARGET_NAME} odbc)
endif()
install(TARGETS ${TARGET_NAME} DESTINATION ./)
