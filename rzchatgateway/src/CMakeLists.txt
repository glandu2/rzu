cmake_minimum_required(VERSION 2.8)

file(GLOB SOURCE_FILES *.cpp *.h)

set(LIBS rzu)
add_exe(${TARGET_NAME} "${SOURCE_FILES}" "${LIBS}")
