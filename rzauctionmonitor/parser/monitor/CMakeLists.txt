cmake_minimum_required(VERSION 2.8.12)

file(GLOB SOURCE_FILES *.cpp *.h)

add_lib(rzauctionwatcher "${SOURCE_FILES}" rzu rzauctionwriter)
target_include_directories(rzauctionwatcher PUBLIC .)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(rzauctionwatcher
	PRIVATE BUILDING_LIBRZAUCTIONWATCHER
	INTERFACE USING_RZAUCTIONWATCHER_SHARED
  )
endif()

add_exe(rzpipelinetest test/test.cpp rzauctionwatcher)
