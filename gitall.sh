#!/bin/sh


echo ----libuv: && cd libuv && git $*
echo ----librzu: && cd ../librzu && git $*
echo ----gtest: && cd ../gtest && git $*
echo ----rztest: && cd ../rztest && git $*
echo ----zlib: && cd ../zlib && git $*
echo ----rzauth: && cd ../rzauth && git $*
echo ----rzbenchauth: && cd ../rzbenchauth && git $*
echo ----rzgame: && cd ../rzgame && git $*
echo ----rzgamereconnect: && cd ../rzgamereconnect && git $*
echo ----rzchatgateway: && cd ../rzchatgateway && git $*
echo ----rzplayercount: && cd ../rzplayercount && git $*
echo ----rzlog: && cd ../rzlog && git $*
echo ----rzfilter: && cd ../rzfilter && git $*

echo ----rzu-parent: && git $*

