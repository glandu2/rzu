cmake_minimum_required(VERSION 2.8.12)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/lua" lua)

file(GLOB SOURCE_FILES *.cpp *.h)

add_module(rzfilter_lua_module "${SOURCE_FILES}" rzu rzfilter_interface lua)
get_target_property(USE_PIC rzfilter_lua_module POSITION_INDEPENDENT_CODE)
set_target_properties(lua PROPERTIES POSITION_INDEPENDENT_CODE ${USE_PIC})
set_target_properties(rzfilter_lua_module PROPERTIES PREFIX "")
