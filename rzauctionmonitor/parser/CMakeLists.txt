cmake_minimum_required(VERSION 2.8.12)

add_exe(rzauctionupgrade "auctions_upgrade.cpp;AuctionSQLWriter.cpp;AuctionSQLWriter.h" rzauctionwriter)
add_exe(rzauctionsql "auctions_to_sql.cpp;AuctionSQLWriter.cpp;AuctionSQLWriter.h" rzu rzauctionwriter)
add_exe(rzauctionjson auctions_to_json.cpp rzu rzauctionwriter)
#add_exe(rzauctionaggregator auctions_aggregator.cpp rzu rzauctionwriter)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/monitor" monitor)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/monitoring_tools" monitoring_tools)
