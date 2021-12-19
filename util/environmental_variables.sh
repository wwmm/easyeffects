#!/bin/sh

export GSETTINGS_SCHEMA_DIR=../../data/schemas/
export G_MESSAGES_DEBUG=easyeffects
export GST_DEBUG_FILE=/tmp/gst_ee_log.txt
export ASAN_OPTIONS="detect_stack_use_after_return=1:check_initialization_order=1"

alias ee_valgrind='valgrind --suppressions=/usr/share/gtk-4.0/valgrind/gtk.supp --suppressions=/usr/share/glib-2.0/valgrind/glib.supp --leak-check=full --show-leak-kinds=definite'
