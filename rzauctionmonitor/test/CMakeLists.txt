cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_rztest(${TARGET_NAME} "${SOURCE_FILES}" rzauctionwriter)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_compile_definitions(${TARGET_NAME}_test PUBLIC NOMINMAX)
endif()
