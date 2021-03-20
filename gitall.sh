#!/bin/sh


echo ----libuv: && cd libuv && git "$@"
echo ----libiconv: && cd ../libiconv && git "$@"
echo ----liblua: && cd ../liblua && git "$@"
echo ----gtest: && cd ../gtest && git "$@"
echo ----zlib: && cd ../zlib && git "$@"

echo ----rzu: && cd .. && git "$@"

