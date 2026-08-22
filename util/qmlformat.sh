#!/usr/bin/env bash

set -euo pipefail

if [[ ! -f .qmlformat.ini ]]; then
    echo "ERROR: Run this in the root directory of the repository where the .qmlformat.ini file is"
    exit 1
fi

# unlike qmllint, this can also be used on js files, not just qml files, so glob everything in the directory

if command -v qmlformat-qt6 &>/dev/null; then
    QMLFORMAT="qmlformat-qt6"
elif [[ -f /usr/lib/qt6/bin/qmlformat ]]; then
    QMLFORMAT="/usr/lib/qt6/bin/qmlformat"
elif command -v qmlformat &>/dev/null; then
    QMLFORMAT="qmlformat"
else
    echo "ERROR: Could not find qmlformat qt6"
    exit 1
fi

echo "Using qmlformat at: $QMLFORMAT"
$QMLFORMAT src/contents/ui/* --inplace "$@" || {
    echo "qmlformat returned $?. This is ignored as Qt 6.7 qmlformat has known bugs returning 1. git diff will catch unformatted files."
}
