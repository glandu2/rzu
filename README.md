rzu emu
=======

rzemu is unified repository containing several tools and servers for various things.

These servers and tool sub-projects are:
* [rzauth](rzauth): Complete auth emu.
* [rzbenchauth](rzbenchauth): auth benchmark tool.
* [rzchatgateway](rzchatgateway): IRC <-> in-game chat gateway.
* [rzgamereconnect](rzgamereconnect): GS autoreconnect to auth tool.
* [rzauctionmonitor](rzauctionmonitor): Auctions monitoring tools, parsers and importer to a SQL database.
* [rzbenchlog](rzbenchlog): log server benchmark tool.
* [rzclientreconnect](rzclientreconnect) Tool to connect a client with multiple account at once, and allowing reconnecting them without closing the client.
* [rzfilter](rzfilter) Packet filter tool, allowing to parse and/or modify them using lua scripts.
* [rzgame](rzgame) Incomplete game server emu.
* [rzlog](rzlog) Log server (to a file or SQL database) (potentially incomplete).
* [rztest](rztest) Library used to implement unit tests for rzemu tools and servers.
* [rztestgs](rztestgs) Incomplete game server tester tool.
* [librzu](librzu) Library containing common stuff to all servers and tools.

External dependencies (as git submodules):
* libuv: Network and misc library allowing to be fully portable between Windows, Linux and other OSes.
* zlib: Compression library
* gtest: Google test, used for unit tests
* libiconv: Charset conversion library
* liblua: LUA VM library

How to compile
--------------

To compile this project, you need [OpenSSL library](http://slproweb.com/products/Win32OpenSSL.html) (Windows: [x86](http://slproweb.com/download/Win32OpenSSL-1_0_2a.exe), [x64](http://slproweb.com/download/Win64OpenSSL-1_0_2a.exe)) and [cmake](http://www.cmake.org/download/#latest).
Then (with Visual Studio) :
```
git clone --recursive https://github.com/glandu2/rzu.git
cd rzu
mkdir build
cd build
cmake ..
cmake --build .
```

Then a Visual Studio solution wil be generated in build/ directory.

