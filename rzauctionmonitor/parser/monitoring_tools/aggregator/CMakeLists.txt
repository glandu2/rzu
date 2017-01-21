cmake_minimum_required(VERSION 2.8.12)

file(GLOB SOURCE_FILES *.cpp *.h)

add_exe(rzauctionaggregator "${SOURCE_FILES}" rzu rzauctionwriter rzauctionwatcher)
