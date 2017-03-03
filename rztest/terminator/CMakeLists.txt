cmake_minimum_required(VERSION 2.8.12)

file(GLOB SOURCE_FILES *.cpp *.h)

add_exe_no_install(rzterminator "${SOURCE_FILES}" rzu rztest)
