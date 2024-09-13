#!/bin/sh

# Validation
[ ! -d "./cross_build" ] && ( echo "Run this script from the project root directory" && exit )
[ -z "$(find ./cross_build/linux64/ -name '*.AppImage' | head -1)" ] && ( echo "Output files don't exist yet" && exit )

# Cleanup
rm ./cross_build/linux64/linuxdeploy-*.AppImage

# Move to dist
mv ./cross_build/linux64/*.AppImage ./dist/
