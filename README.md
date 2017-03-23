# PulseEffects

Audio effects for Pulseaudio applications

![](pulseeffects/images/pulseeffects_main_window.png)
![](pulseeffects/images/pulseeffects_eq_menu.png)
![](pulseeffects/images/pulseeffects_reverb_menu.png)

Effects order:

1. Fast Lookahead Limiter
2. Reverberation
3. Equalizer

Required libraries:

- Python 3
- PyGobject 3
- Gtk 3.18 or above
- Gstreamer, Gstreamer Plugins Good, Gstreamer Plugins Bad and Gstreamer Python (version 1.0 or above for all of them)
- swh-plugins from Ladspa

Execution:

	python pulse_effects
