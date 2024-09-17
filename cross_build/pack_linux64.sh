#!/bin/sh

set -e

# Validation
[ ! -d "./cross_build" ] && echo "Run this script from the project root directory" && exit
[ -z "$QSP_RELEASE_VER" ] && echo "QSP_RELEASE_VER isn't specified" && exit

# Build
mkdir -p ./cross_build/linux64

IMAGE=dockbuild/ubuntu1804-gcc7
SCRIPT=cross_build/build_linux64.sh

SSH_DIR="$HOME/.ssh"
HOST_VOLUMES="-v $SSH_DIR:/home/$(id -un)/.ssh"
USER_IDS="-e BUILDER_UID=$( id -u ) -e BUILDER_GID=$( id -g ) -e BUILDER_USER=$( id -un ) -e BUILDER_GROUP=$( id -gn )"
APP_ARGS="-e APP_VERSION=$QSP_RELEASE_VER"
tty -s && TTY_ARGS="-ti" || TTY_ARGS=""

docker run --rm \
  -v "$(pwd)":/work \
  $TTY_ARGS \
  $HOST_VOLUMES \
  $USER_IDS \
  $APP_ARGS \
  $IMAGE "/work/$SCRIPT"

# Move to dist
mv ./cross_build/linux64/packages/*.rpm "./dist/qspgui-$QSP_RELEASE_VER-linux64.rpm"
mv ./cross_build/linux64/packages/*.deb "./dist/qspgui-$QSP_RELEASE_VER-linux64.deb"
mv ./cross_build/linux64/packages/*.tar.gz "./dist/qspgui-$QSP_RELEASE_VER-linux64.tar.gz"
