#!/bin/sh

[ ! -d "./cross_build" ] && echo "Run this script from the project root directory" && exit

./cross_build/dockcross-windows-static-x86 cmake -S . -B ./cross_build/win32 -GNinja -DCMAKE_INSTALL_PREFIX=./cross_build/win32/out -DCMAKE_BUILD_TYPE=Release
./cross_build/dockcross-windows-static-x86 cmake --build ./cross_build/win32 --target install
