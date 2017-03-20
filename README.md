# PulseEffects

Stereo equalizer and reverberation for Pulseaudio applications

![](images/pulseeffects_main_window.png)
![](images/pulseeffects_eq_menu.png)
![](images/pulseeffects_reverb_menu.png) 

Effects order:

1. Fast Lookahead Limiter
2. Reverberation
3. Equalizer

Required libraries:

- Python 3 
- PyGobject 3
- Gtk 3.18 or above
- Gstreamer, Gstreamer Plugins Good, Gstreamer Plugins Bad and Gstreamer Python (version 1.0 or above for all of them)

Execution:

	python pulse_effects.py