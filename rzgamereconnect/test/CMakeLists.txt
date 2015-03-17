cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

#add_executable(${TARGET_NAME}_test ${SOURCE_FILES})
#target_link_libraries(${TARGET_NAME}_test ${TARGET_NAME}_lib gtest)
#add_test(${TARGET_NAME}_test "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}_test")

add_rztest(${TARGET_NAME} ${SOURCE_FILES})
