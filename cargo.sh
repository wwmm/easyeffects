#!/bin/sh

export MESON_BUILD_ROOT="$1"
export MESON_SOURCE_ROOT="$2"
export CARGO_TARGET_DIR="$MESON_BUILD_ROOT"/target

RESOURCE_FILE="$MESON_BUILD_ROOT"/data/resources.gresource
RESOURCE_DESTINATION="$MESON_SOURCE_ROOT"/src/ui

cp -v $RESOURCE_FILE $RESOURCE_DESTINATION

if [[ $4 = "debugoptimized" ]]
then
    echo "DEBUG MODE"
    cargo build --manifest-path \
        "$MESON_SOURCE_ROOT"/Cargo.toml --verbose && \
        cp "$CARGO_TARGET_DIR"/debug/pulseeffects $3
else
    echo $4
    echo "RELEASE MODE"
    cargo build --manifest-path \
        "$MESON_SOURCE_ROOT"/Cargo.toml --release && \
        cp "$CARGO_TARGET_DIR"/release/pulseeffects $3
fi
