#!/bin/sh

set -e

# Validation
[ ! -d "./build_packages" ] && echo "Run this script from the project root directory" && exit
[ -z "$RELEASE_VER" ] && echo "RELEASE_VER isn't specified" && exit

# Build
mkdir -p ./build_packages/linux64

IMAGE=dockbuild/ubuntu1804-gcc7
SCRIPT=build_packages/build_linux64.sh

SSH_DIR="$HOME/.ssh"
HOST_VOLUMES="-v $SSH_DIR:/home/$(id -un)/.ssh"
USER_IDS="-e BUILDER_UID=$( id -u ) -e BUILDER_GID=$( id -g ) -e BUILDER_USER=$( id -un ) -e BUILDER_GROUP=$( id -gn )"
APP_ARGS="-e APP_VERSION=$RELEASE_VER"
tty -s && TTY_ARGS="-ti" || TTY_ARGS=""

docker run --rm \
  -v "$(pwd)":/work \
  $TTY_ARGS \
  $HOST_VOLUMES \
  $USER_IDS \
  $APP_ARGS \
  $IMAGE "/work/$SCRIPT"

# Move to dist
mv ./build_packages/linux64/packages/*.rpm "./dist/qspgui-$RELEASE_VER-linux64.rpm"
mv ./build_packages/linux64/packages/*.deb "./dist/qspgui-$RELEASE_VER-linux64.deb"
mv ./build_packages/linux64/packages/*.tar.gz "./dist/qspgui-$RELEASE_VER-linux64.tar.gz"
