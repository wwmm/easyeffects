#!/usr/bin/env bash

# using git log to view the changes that have been done since the tag given as argument

read -r -e -p "Write the last release version(example 6.2.6): " LAST_RELEASE

echo ""
echo "Changes done since $LAST_RELEASE:"
echo ""

git log v$LAST_RELEASE...HEAD --pretty=format:'%H %s'