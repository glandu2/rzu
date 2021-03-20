#!/bin/sh

find rz* librzu \( -name '*.cpp' -o -name '*.h' -o -name '*.inl' \) | xargs clang-format -style=file -i
