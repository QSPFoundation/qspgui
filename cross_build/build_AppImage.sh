#!/bin/sh

sudo apt-get update
sudo apt-get install -y \
  libgtk-3-dev \
  wget \
  fuse

BUILD_DIR=/work/cross_build/linux64

cmake -S /work -B $BUILD_DIR -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD_DIR --parallel=$(nproc) --target install DESTDIR=$BUILD_DIR/AppDir

cd $BUILD_DIR || echo "Can't switch to $BUILD_DIR" || exit

wget -c "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh"
wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-gtk.sh

export LD_LIBRARY_PATH=$(find $BUILD_DIR/AppDir -type d -printf ":%p")
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin gtk --output appimage
