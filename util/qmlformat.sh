#!/usr/bin/env bash

set -euo pipefail

if [[ ! -f .qmlformat.ini ]]; then
    echo "ERROR: Run this in the root directory of the repository where the .qmlformat.ini file is"
    exit 1
fi

# unlike qmllint, this can also be used on js files, not just qml files, so glob everything in the directory

# for some reason the arch qt6-declarative package doesn't add this to the normally searched directories
if [[ -f /usr/lib/qt6/bin/qmlformat ]]; then
    /usr/lib/qt6/bin/qmlformat src/contents/ui/* --inplace "$@"
# for fedora
elif command -v qmlformat-qt6 --help &>/dev/null; then
    qmlformat-qt6 src/contents/ui/* --inplace "$@"
else
    echo "ERROR: Could not find qmlformat qt6"
    exit 1
fi
