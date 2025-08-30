#!/usr/bin/env bash

set -euo pipefail

if [[ ! -f .qmllint.ini ]]; then
    echo "ERROR: Run this in the root directory of the repository where the .qmllint.ini file is"
    exit 1
fi

# for some reason the arch qt6-declarative package doesn't add this to the normally searched directories
if [[ -f /usr/lib/qt6/bin/qmllint ]]; then
    /usr/lib/qt6/bin/qmllint src/contents/ui/*.qml "$@"
# for fedora
elif command -v qmllint-qt6 --help &>/dev/null; then
    qmllint-qt6 src/contents/ui/*.qml "$@"
else
    echo "ERROR: Could not find qmllint qt6"
    exit 1
fi
