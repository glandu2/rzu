cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
set(LIBS rzu ${OPENSSL_LIBRARIES})
add_rztest(${TARGET_NAME} "${SOURCE_FILES}" "${LIBS}")
