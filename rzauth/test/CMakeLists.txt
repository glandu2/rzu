cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

find_package(OpenSSL REQUIRED)
add_rztest(${TARGET_NAME} "${SOURCE_FILES}" rzu ${OPENSSL_LIBRARIES})
target_include_directories(${TARGET_NAME}_test PRIVATE ${OPENSSL_INCLUDE_DIR})
