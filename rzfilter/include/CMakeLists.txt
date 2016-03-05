cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.h)

add_library(rzfilter_interface INTERFACE)
target_link_libraries(rzfilter_interface INTERFACE rzu)
target_include_directories(rzfilter_interface INTERFACE "${CMAKE_CURRENT_LIST_DIR}")

#Only add headers to target if support (the purpose of this is just to add them in the IDE)
if((CMAKE_VERSION VERSION_GREATER "3.1") OR (CMAKE_VERSION VERSION_EQUAL "3.1"))
  target_sources(rzfilter_interface INTERFACE ${SOURCE_FILES})
endif()
