cmake_minimum_required(VERSION 2.8.12)

#add_exe(rzauctiondiff auctions_diff.cpp)
add_exe(rzauctionupgrade auctions_upgrade.cpp rzauctionwriter)
add_exe(rzauctionparser auctions_to_sql.cpp rzu rzauctionwriter)

if(NOT WIN32)
  add_exe(rzauctiondump auctions_dump.cpp crypto)
endif()
