cmake_minimum_required(VERSION 2.8.12)

add_lib(rzfilter_module "PacketFilter.cpp;PacketFilter.h" rzu rzfilter_interface)
add_lib(rzfilter_game_version_converter "GamePacketConverterFilter.cpp;GamePacketConverterFilter.h" rzu rzfilter_interface)
add_lib(rzfilter_auth_version_converter "AuthPacketConverterFilter.cpp;AuthPacketConverterFilter.h" rzu rzfilter_interface)

set_target_properties(rzfilter_module PROPERTIES PREFIX "")
set_target_properties(rzfilter_game_version_converter PROPERTIES PREFIX "")
set_target_properties(rzfilter_auth_version_converter PROPERTIES PREFIX "")
