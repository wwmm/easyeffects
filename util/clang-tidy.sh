#!/usr/bin/env bash
set -e
# write a custom script for ease of use, meson clang-tidy target can't do fixes, and also has somewhat broken threading

if ! meson introspect --projectinfo &> /dev/null; then
    echo "Error, meson introspect command failed, make sure you are in a meson build directory"
    exit 1
fi

if [[ "$PWD" == "" ]]; then
    echo "Error, \$PWD is empty"
    exit 1
fi

# save current dir and get repo dir
# we need an absolute path without any .. in them otherwise these various layers of scripts get confused
BUILD_DIR="$PWD"

cd ..
REPO_DIR="$PWD"

# back to build dir
cd "$BUILD_DIR"

# only scan c++ files in src, we don't want to scan the generated easyeffects-resources.c file
# it is fine to use -fix everywhere, since it will still exit with error even if errors were all automatically fixed.
# however we do not use -fix since it is safer to just manually fix things
python3 ../util/run-clang-tidy.py -p . -config-file=../.clang-tidy "$REPO_DIR"/src/*.cpp
