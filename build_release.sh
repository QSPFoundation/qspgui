#!/bin/sh

export QSP_RELEASE_VER="${1:-latest}"

mkdir -p dist

echo "Preparing Win32 release $QSP_RELEASE_VER"
./cross_build/pack_win32.sh

echo "Preparing Linux64 release $QSP_RELEASE_VER"
./cross_build/pack_linux64.sh

echo "Preparing Linux64 AppImage release $QSP_RELEASE_VER"
./cross_build/pack_linux64_AppImage.sh
