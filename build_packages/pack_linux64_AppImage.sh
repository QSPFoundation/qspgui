#!/bin/sh

set -e

# Validation
[ ! -d "./build_packages" ] && echo "Run this script from the project root directory" && exit
[ -z "$QSP_RELEASE_VER" ] && echo "QSP_RELEASE_VER isn't specified" && exit

CMAKE_VER=$(echo "$QSP_RELEASE_VER" | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')

# Build
mkdir -p ./build_packages/linux64_AppImage

IMAGE=dockbuild/ubuntu1804-gcc7
SCRIPT=build_packages/build_AppImage.sh

SSH_DIR="$HOME/.ssh"
HOST_VOLUMES="-v $SSH_DIR:/home/$(id -un)/.ssh"
USER_IDS="-e BUILDER_UID=$( id -u ) -e BUILDER_GID=$( id -g ) -e BUILDER_USER=$( id -un ) -e BUILDER_GROUP=$( id -gn )"
APP_ARGS="-e APP_VERSION=$CMAKE_VER"
# Allow usage of fuse
DOCKER_OPTS="--cap-add SYS_ADMIN --device /dev/fuse --security-opt apparmor:unconfined"
tty -s && TTY_ARGS="-ti" || TTY_ARGS=""

docker run --rm \
  -v "$(pwd)":/work \
  $TTY_ARGS \
  $HOST_VOLUMES \
  $DOCKER_OPTS \
  $USER_IDS \
  $APP_ARGS \
  $IMAGE "/work/$SCRIPT"

# Cleanup
rm ./build_packages/linux64_AppImage/linuxdeploy-*.AppImage

# Move to dist
mv ./build_packages/linux64_AppImage/*.AppImage "./dist/QSP_Classic-$QSP_RELEASE_VER-x86_64.AppImage"
