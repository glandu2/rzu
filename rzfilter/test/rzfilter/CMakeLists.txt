cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_rztest(${TARGET_NAME} "${SOURCE_FILES}" rzu)

configure_file(data/rzfilter-test.opt ${CMAKE_BINARY_DIR}/rzfilter-test.opt)
configure_file(data/rzfilter.lua ${CMAKE_BINARY_DIR}/rzfilter.lua)
