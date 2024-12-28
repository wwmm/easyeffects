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
    # replacing schema enums, but the property names and the labels should be filled manually afterwards
    sed -r -z 's|"\s+value="[0-9]+"\s+/>\s+<value nick="|,|g' |
    sed -r 's/<value\s+nick="/<default>/g' |
    sed -r 's|"\s+value="[0-9]+"\s+/>|</default>|g' |
    sed -r 's/<enum[^>]+>/    <entry name="Labels" type="StringList">/g' |
    sed -r 's|enum="[^"]+">|type="Int">\n                <label></label>|g' |
    sed -r 's|</enum>|</entry>|g' |
    sed -r '/<([?]xml|[/]?schema(list)?)[^>]*>/d' | # removing lines with unneeded tags
    sed '$ a \    </group>' | # append closing tags
    sed '$ a </kcfg>' |
    sed '1 i \
<?xml version="1.0" encoding="UTF-8"?> \
<kcfg xmlns="http://www.kde.org/standards/kcfg/1.0" \
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \
    xsi:schemaLocation="http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/standards/kcfg/1.0/kcfg.xsd"> \
    <kcfgfile name="easyeffectsrc" /> \
    <group name="imported">' |
    sed -r 's|<range\s+min="([-+]?[0-9.]*)"\s+max="([-+]?[0-9.]*)"\s*/>|    <min>\1</min>\n                <max>\2</max>|g' |
    sed -r 's/(<default[^>]*>)/    \1/g' > $OUTPUT_FILE
