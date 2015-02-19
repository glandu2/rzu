#!/bin/sh

echo rzu-parent: && git $*

echo libuv: && cd libuv && git $*
echo librzu: && cd ../librzu && git $*
echo rzauth: && cd ../rzauth && git $*
echo rzbenchauth: && cd ../rzbenchauth && git $*
echo rzgamereconnect: && cd ../rzgamereconnect && git $*
echo rzchatgateway: && cd ../rzchatgateway && git $*
echo rzplayercount: && cd ../rzplayercount && git $*
echo VStructGen: && cd ../VStructGen && git $*
#echo RappelzClient: && cd ../RappelzClient && git $*

