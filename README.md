rzu-parent
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

To compile this project, you need OpenSSL library.
Then (with Visual Studio) :
```
git clone --recursive https://github.com/glandu2/rzu-parent.git
cd rzu-parent
mkdir build
cd build
cmake ..
```

Then a Visual Studio solution wil be generated in build/ directory.

Project for the [ElitePvPers](http://www.elitepvpers.com/forum/rappelz-private-server/) community,
[original thread](http://www.elitepvpers.com/forum/rappelz-private-server/3704022-release-rappelz-auth-emu-v4-0-a.html)

