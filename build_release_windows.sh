#!/bin/sh

export RELEASE_VER="${1:-0.0.0}"

mkdir -p dist

echo "Preparing Win32 release $RELEASE_VER"
./build_packages/pack_win32.sh
