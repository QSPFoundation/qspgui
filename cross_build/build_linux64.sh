#!/bin/sh

[ ! -d "./cross_build" ] && ( echo "Run this script from the project root directory" && exit )

mkdir -p ./cross_build/linux64

IMAGE=dockbuild/ubuntu1804-gcc7
SCRIPT=cross_build/build_AppImage.sh

SSH_DIR="$HOME/.ssh"
HOST_VOLUMES="-v $SSH_DIR:/home/$(id -un)/.ssh"
USER_IDS="-e BUILDER_UID=$( id -u ) -e BUILDER_GID=$( id -g ) -e BUILDER_USER=$( id -un ) -e BUILDER_GROUP=$( id -gn )"
# Allow usage of fuse
DOCKER_OPTS="--cap-add SYS_ADMIN --device /dev/fuse --security-opt apparmor:unconfined"

docker run --rm -ti \
  -v "$(pwd)":/work \
  $HOST_VOLUMES \
  $DOCKER_OPTS \
  $USER_IDS \
  $IMAGE "/work/$SCRIPT"