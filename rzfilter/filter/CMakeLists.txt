cmake_minimum_required(VERSION 2.8.12)

add_lib(rzfilter_module "PacketFilter.cpp;PacketFilter.h" rzu rzfilter_interface)
add_lib(rzfilter_version_converter "PacketConverterFilter.cpp;PacketConverterFilter.h" rzu rzfilter_interface)

set_target_properties(rzfilter_module PROPERTIES PREFIX "")
set_target_properties(rzfilter_version_converter PROPERTIES PREFIX "")
