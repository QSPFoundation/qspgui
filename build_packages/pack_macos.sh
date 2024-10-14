#!/bin/sh

set -e

# Validation
[ ! -d "./build_packages" ] && echo "Run this script from the project root directory" && exit
[ -z "$QSP_RELEASE_VER" ] && echo "QSP_RELEASE_VER isn't specified" && exit

CMAKE_VER=$(echo "$QSP_RELEASE_VER" | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')

# Build
mkdir -p ./build_packages/macos

REL_BUILD_DIR=./build_packages/macos

cmake -S . -B $REL_BUILD_DIR \
  -DAPP_VERSION="$CMAKE_VER" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCPACK_OUTPUT_FILE_PREFIX=$REL_BUILD_DIR/packages \
  -DCPACK_GENERATOR="DragNDrop" \
  -DCMAKE_INSTALL_PREFIX=$REL_BUILD_DIR/out \
  -DCMAKE_BUILD_TYPE=Release

cmake --build $REL_BUILD_DIR --parallel $(sysctl -n hw.ncpu)
cmake --install $REL_BUILD_DIR --component Main --strip
cpack -B $REL_BUILD_DIR --config $REL_BUILD_DIR/CPackConfig.cmake

# Move to dist
mv ./build_packages/macos/packages/*.dmg "./dist/qspgui-$QSP_RELEASE_VER-universal.dmg"
