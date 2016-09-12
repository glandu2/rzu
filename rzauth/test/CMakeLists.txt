cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

find_package(OpenSSL REQUIRED)
add_rztest(${TARGET_NAME} "${SOURCE_FILES}" rzu ${OPENSSL_LIBRARIES})
target_include_directories(${TARGET_NAME}_test PRIVATE ${OPENSSL_INCLUDE_DIR})

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../../rzgamereconnect)
	set(AUTH_TEST_HAS_RZGAMERECONNECT "true")
	configure_file(data/auth-test.opt ${CMAKE_BINARY_DIR}/auth-test.opt @ONLY)
else()
	set(AUTH_TEST_HAS_RZGAMERECONNECT "false")
	configure_file(data/auth-test.opt ${CMAKE_BINARY_DIR}/auth-test.opt)
endif()
