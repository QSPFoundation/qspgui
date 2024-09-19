#!/bin/sh

set -e

sudo apt-get update
sudo apt-get install -y \
  libgtk-3-dev \
  rpm

REL_BUILD_DIR=./cross_build/linux64

cmake -S . -B $REL_BUILD_DIR \
  -DAPP_VERSION="$APP_VERSION" \
  -DCPACK_OUTPUT_FILE_PREFIX=$REL_BUILD_DIR/packages \
  -DCPACK_GENERATOR="TGZ;DEB;RPM" \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DCMAKE_BUILD_TYPE=Release

cmake --build $REL_BUILD_DIR --parallel $(nproc)
cpack -B $REL_BUILD_DIR --config $REL_BUILD_DIR/CPackConfig.cmake
