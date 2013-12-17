cmake_minimum_required(VERSION 2.8)

file(GLOB SOURCE_FILES *.cpp *.h)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} RappelzLib)
target_link_libraries(${TARGET_NAME} crypto uv odbc)
  target_link_libraries(${TARGET_NAME} m)
  target_link_libraries(${TARGET_NAME} pthread)
  target_link_libraries(${TARGET_NAME} dl)
  target_link_libraries(${TARGET_NAME} rt)
install(TARGETS ${TARGET_NAME} DESTINATION ./)
