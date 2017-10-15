cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_module(rzfilter_json "${SOURCE_FILES}" rzu rzfilter_interface)
set_target_properties(rzfilter_json PROPERTIES PREFIX "")
