#!/bin/sh

export QSP_RELEASE_VER="${1:-0.0.0}"

mkdir -p dist

echo "Preparing MacOS release $QSP_RELEASE_VER"
./build_packages/pack_macos.sh
