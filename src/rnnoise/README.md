# PulseEffects RNNoise Noise Suppression

Simple Noise Suppression plugin based on the library [RNNoise](https://gitlab.xiph.org/xiph/rnnoise) by Xiph Foundation.

You can test it from command line executing:

`gst-launch-1.0 -v audiotestsrc ! pernnoise model-path=full_path_to_optional_model_file ! pulsesink`
