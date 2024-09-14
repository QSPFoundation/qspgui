#!/bin/sh

# Validation
[ ! -d "./cross_build" ] && ( echo "Run this script from the project root directory" && exit )
[ ! -d "./cross_build/win32/out" ] && ( echo "Output directory doesn't exist yet" && exit )

# Cleanup
rm ./cross_build/win32/out/bin/onig-config

# Packaging
(cd ./cross_build/win32/out/bin/; zip -r qspgui_win32.zip ./)

# Move to dist
mv ./cross_build/win32/out/bin/qspgui_win32.zip "./dist/qspgui-$QSP_RELEASE_VER-win32.zip"
