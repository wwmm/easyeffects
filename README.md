# PulseEffects

Limiter, reverberation and stereo equalizer effects for Pulseaudio applications

![](PulseEffects/images/pulseeffects_main_window.png)
![](PulseEffects/images/pulseeffects_eq_menu.png)
![](PulseEffects/images/pulseeffects_reverb_menu.png)

Effects order:

1. Fast Lookahead Limiter
2. SC4 Compressor
3. Gstreamer Freeverb
4. Caps 10 Bands Equalizer

Required libraries:

- Python 3
- PyGobject 3
- Gtk 3.18 or above
- Gstreamer, Gstreamer Plugins Good, Gstreamer PLugins Bad and Gstreamer Python
 (version 1.0 or above for all of them)
- swh-plugins and caps from Ladspa

Arch Linux package:

[https://aur.archlinux.org/packages/pulseeffects/](https://aur.archlinux.org/packages/pulseeffects/)
