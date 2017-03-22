cmake_minimum_required(VERSION 2.8.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_module(rzfilter_version_converter "${SOURCE_FILES}" rzu rzfilter_interface)
set_target_properties(rzfilter_version_converter PROPERTIES PREFIX "")
