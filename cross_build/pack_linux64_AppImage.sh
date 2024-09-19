#!/bin/sh

set -e

# Validation
[ ! -d "./cross_build" ] && echo "Run this script from the project root directory" && exit
[ -z "$QSP_RELEASE_VER" ] && echo "QSP_RELEASE_VER isn't specified" && exit

CMAKE_VER=$(echo "$QSP_RELEASE_VER" | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')

# Build
mkdir -p ./cross_build/linux64_AppImage

IMAGE=dockbuild/ubuntu1804-gcc7
SCRIPT=cross_build/build_AppImage.sh

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
rm ./cross_build/linux64_AppImage/linuxdeploy-*.AppImage

# Move to dist
mv ./cross_build/linux64_AppImage/*.AppImage "./dist/QSP_Classic-$QSP_RELEASE_VER-x86_64.AppImage"
