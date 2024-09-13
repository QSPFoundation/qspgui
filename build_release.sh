#!/bin/sh

mkdir -p dist

echo "Preparing Win32 release"
./cross_build/build_win32.sh && ./cross_build/pack_win32.sh

echo "Preparing Linux64 release"
./cross_build/build_linux64.sh && ./cross_build/pack_linux64.sh
