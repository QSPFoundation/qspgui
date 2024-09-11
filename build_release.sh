#!/bin/sh

mkdir -p dist
./cross_build/build_win32.sh && ./cross_build/pack_win32.sh
