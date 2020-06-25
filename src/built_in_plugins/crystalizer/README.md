# PulseEffects Crystalizer

Simple plugin useful to add more dynamic range to songs that were overly
compressed. It is based on the [FFMPEG Crystalizer plugin code](https://git.ffmpeg.org/gitweb/ffmpeg.git/blob_plain/HEAD:/libavfilter/af_crystalizer.c).

You can test this plugin from command line executing:

`gst-launch-1.0 -v audiotestsrc ! pecrystalizer ! pulsesink`
