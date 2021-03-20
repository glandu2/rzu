#!/bin/sh

if [ -z "$1" ]; then
	echo "Usage: $0 x.y" >&2
	exit 1
fi

VERSION="$1"

sed -i -r 's/set\(GIT_VERSION "[0-9\.]+"/set\(GIT_VERSION "'"${VERSION}"'"/' CMakeLists.txt
sed -i -r "s/version: [0-9\.]+\.\{build\}/version: ${VERSION}.{build}/" appveyor.yml
