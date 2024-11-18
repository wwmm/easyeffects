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
    sed -r 's/(<key\s+name="[a-z0-9]*)-([a-z])/\1\u\2/g' | # removing "-" and putting the next character in upper case
    sed -r 's/key/entry/' |
    sed -r 's/type="b"/type="Bool"/' |
    sed -r 's/type="i"/type="Int"/' |
    sed -r 's/type="d"/type="Double"/' |
    sed -r 's/type="s"/type="String"/' |
    # replacing schema enums, but they should be moved in the entries list after the script execution,
    # then empty choice and name properties should be filled with new values
    sed -r 's/<enum[^>]+>/\t<entry name="" type="Enum">\n\t\t\t<choices>/g' |
    sed -r 's/<value\s+nick="/\t\t<choice name="">\n\t\t\t\t\t<label>/g' |
    sed -r 's/"\s+value="[^"]+"\s*\/>/<\/label>\n\t\t\t\t<\/choice>/g' |
    sed -r 's/<\/enum>/\t\t<\/choices>\n\t\t<default><\/default>\n\t\t<\/entry>/g' |
    sed -E '/<([?]xml|[/]?schema(list)?)[^>]*>/d' | # removing lines with unneeded tags
    sed '$ a \\t</group>' | # append closing tags
    sed '$ a </kcfg>' |
    sed '1 i \
<?xml version="1.0" encoding="UTF-8"?> \
<kcfg xmlns="http://www.kde.org/standards/kcfg/1.0" \
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \
    xsi:schemaLocation="http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/standards/kcfg/1.0/kcfg.xsd"> \
    <kcfgfile name="easyeffectsrc" /> \
    <group name="imported">' |
    sed -r 's/(<entry[^>]*>)/\1\n\t\t\t\t<label><\/label>/g' |
    sed -r 's/<range min="([-+]?[0-9.]*)" max="([-+]?[0-9.]*)"\s*\/>/\t<min>\1<\/min>\n\t\t\t\t<max>\2<\/max>/g' |
    sed -r 's/(<default[^>]*>)/\t\1/g' > $OUTPUT_FILE
