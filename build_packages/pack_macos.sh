#!/bin/sh

set -e

# Validation
[ ! -d "./build_packages" ] && echo "Run this script from the project root directory" && exit
[ -z "$RELEASE_VER" ] && echo "RELEASE_VER isn't specified" && exit

CMAKE_VER=$(echo "$RELEASE_VER" | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')

# Build
mkdir -p ./build_packages/macos

REL_BUILD_DIR=./build_packages/macos

cmake -S . -B $REL_BUILD_DIR \
  -DAPP_VERSION="$CMAKE_VER" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11 \
  -DCPACK_OUTPUT_FILE_PREFIX=$REL_BUILD_DIR/packages \
  -DCPACK_GENERATOR="DragNDrop" \
  -DCMAKE_INSTALL_PREFIX=$REL_BUILD_DIR/out \
  -DCMAKE_BUILD_TYPE=Release

cmake --build $REL_BUILD_DIR --parallel $(sysctl -n hw.ncpu)
cmake --install $REL_BUILD_DIR --component Main --strip
cpack -B $REL_BUILD_DIR --config $REL_BUILD_DIR/CPackConfig.cmake

# Move to dist
mv $REL_BUILD_DIR/packages/*.dmg "./dist/qspgui-$RELEASE_VER-universal.dmg"
