#!/bin/bash

set -e

SCHEMA=$1
UTIL_DIR="$PWD"
OUTPUT_FILE=/tmp/schema.kcfg

cd ..
REPO_DIR="$PWD"

cd "$UTIL_DIR"

# s/regexp/replacement/
cat $SCHEMA |
    sed -r 's/-(.)/\u\1/g' | # removing "-" and putting the next character in upper case
    sed -r 's/key/entry/' | 
    sed -r 's/type="b"/type="Bool"/' |
    sed -r 's/type="i"/type="Int"/' |
    sed -r 's/type="s"/type="String"/' |
    sed '1d;2d;3d;$d;' | # removing lines
    sed '$d' |
    sed '$ a \\t</group>' |
    sed '$ a </kcfg>' |
    sed '1 i \
<?xml version="1.0" encoding="UTF-8"?> \
<kcfg xmlns="http://www.kde.org/standards/kcfg/1.0" \
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \
    xsi:schemaLocation="http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/standards/kcfg/1.0/kcfg.xsd"> \
    <kcfgfile name="easyeffectsrc" /> \
    <group name="imported">' |
    sed -r 's/(<entry.*>)/\1\n\t\t\t\t<label><\/label>/g' |
    sed -r 's/<range min="(.*)" max="(.*)".*\/>/\t<min>\1<\/min>\n\t\t\t\t<max>\2<\/max>/g' |
    sed -r 's/(<default.*>)/\t\1/g' > $OUTPUT_FILE