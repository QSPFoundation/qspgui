#!/bin/sh

set -e

# Validation
[ ! -d "./cross_build" ] && echo "Run this script from the project root directory" && exit

# Build
mkdir -p ./cross_build/win32

./cross_build/dockcross-windows-static-x86 cmake -S . -B ./cross_build/win32 -GNinja -DAPP_VERSION="$QSP_RELEASE_VER" -DCMAKE_INSTALL_PREFIX=./cross_build/win32/out -DCMAKE_BUILD_TYPE=Release
./cross_build/dockcross-windows-static-x86 cmake --build ./cross_build/win32 --parallel --target install

# Cleanup
rm ./cross_build/win32/out/bin/onig-config

# Packaging
(cd ./cross_build/win32/out/bin/; zip -r qspgui_win32.zip ./)

# Move to dist
mv ./cross_build/win32/out/bin/qspgui_win32.zip "./dist/qspgui-$QSP_RELEASE_VER-win32.zip"
