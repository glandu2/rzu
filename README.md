rzu
=======

Root project for these sub-projects:
* rzauth (Auth emu)
* rzbenchauth (auth benchmark tool)
* rzchatgateway (IRC <-> in-game chat gateway)
* rzgamereconnect (GS autoreconnect to auth tool)
* librzu
* libuv
* zlib
* rztest
* gtest

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

