#!/usr/bin/env bash
set -e
# write a custom script for ease of use, meson clang-tidy target can't do fixes, and also has somewhat broken threading

if ! meson introspect --projectinfo &> /dev/null; then
    echo "Error, meson introspect command failed, make sure you are in a meson build directory"
    exit 1
fi

# it is fine to use -fix everywhere, since it will still exit with error even if errors were all automatically fixed.
# however we do not use -fix since it is safer to just manually fix things
python3 ../util/run-clang-tidy.py -p . -config-file=../.clang-tidy
