#!/bin/sh

export GSETTINGS_SCHEMA_DIR=../../data/schemas/
export GST_PLUGIN_PATH_1_0=convolver:crystalizer:autogain:adapter
export G_MESSAGES_DEBUG=pulseeffects
export GST_DEBUG_FILE=/tmp/gst_pe_log.txt
