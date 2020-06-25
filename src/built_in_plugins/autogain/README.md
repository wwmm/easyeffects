# PulseEffects autogain

Simple plugin that changes audio gain to match the levels recommended by the
ebur128 standard.

You can test this plugin from command line executing:

`gst-launch-1.0 -v audiotestsrc ! peautogain ! pulsesink`
