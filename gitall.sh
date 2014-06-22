#!/bin/sh

echo RappelzUnified:
git $*
echo libuv:
cd libuv
git $*
echo RappelzLib:
cd ../RappelzLib
git $*
echo RappelzPlayerCount:
cd ../RappelzPlayerCount
git $*
echo RappelzServerAuth:
cd ../RappelzServerAuth
git $*
echo RappelzClient:
cd ../RappelzClient
git $*
echo RappelzAuthBenchmark:
cd ../RappelzAuthBenchmark
git $*

