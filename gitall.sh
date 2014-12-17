#!/bin/sh

echo rzu-parent: && git $*

echo libuv: && cd libuv && git $*
echo librzu: && cd ../librzu && git $*
echo rzauth: && cd ../rzauth && git $*
echo PlayerCount: && cd ../PlayerCount && git $*
echo ChatGateway: && cd ../ChatGateway && git $*
echo rzbenchauth: && cd ../rzbenchauth && git $*
echo VStructGen: && cd ../VStructGen && git $*
#echo RappelzClient: && cd ../RappelzClient && git $*

