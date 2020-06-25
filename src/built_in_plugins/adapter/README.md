# PulseEffects Buffer Size Adapter

Simple plugin that gives output buffers with the desired number of samples. It
is used in PulseEffects to ensure that the number of audio samples in the buffer
is a power of 2. The convolver needs this.

You can test this plugin from command line executing:

`gst-launch-1.0 -v audiotestsrc ! peadapter ! pulsesink`
