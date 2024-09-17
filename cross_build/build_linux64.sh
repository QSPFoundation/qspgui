#!/bin/sh

set -e

sudo apt-get update
sudo apt-get install -y \
  libgtk-3-dev \
  rpm

REL_BUILD_DIR=cross_build/linux64
ABS_BUILD_DIR=/work/$REL_BUILD_DIR

cmake -S /work -B $ABS_BUILD_DIR -DAPP_VERSION="$APP_VERSION" -DCMAKE_INSTALL_PREFIX=$REL_BUILD_DIR/out -DCMAKE_BUILD_TYPE=Release
cmake --build $ABS_BUILD_DIR --parallel=$(nproc) --target install

LIB_PATH=$(find $ABS_BUILD_DIR/out -type d -printf ":%p")
LD_LIBRARY_PATH=$LIB_PATH cpack -B $ABS_BUILD_DIR --config $ABS_BUILD_DIR/CPackConfig.cmake
