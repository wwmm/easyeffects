# PulseEffects

Limiter, compressor, reverberation, stereo equalizer and auto volume effects for
Pulseaudio applications

![](images/pulseeffects_main_window.png)
![](images/pulseeffects_eq_menu.png)
![](images/pulseeffects_reverb_menu.png)

Order of effects applied to applications output:

1. Input Limiter (Ladspa Fast Lookahead Limiter)
2. Auto Volume
3. Stereo Panorama (Gstreamer)
4. Compressor (Ladspa SC4)
5. Freeverb (Gstreamer)
6. Equalizer Input Gain (Gstreamer volume)
7. Butterworth Highpass filter (Gstreamer audiocheblimit)
8. Butterworth Lowpass filter (Gstreamer audiocheblimit)
9. 15 bands Equalizer (Gstreamer)
10. Equalizer Output Gain (Gstreamer volume)
11. Output Limiter (Ladspa Fast Lookahead Limiter)
12. Spectrum Analyzer (Gstreamer)

Since version 2.0.0 PulseEffects is capable of applying effects to microphone
output at the same time it applies them for applications output:

1. Input Limiter (Ladspa Fast Lookahead Limiter)
2. Compressor (Ladspa SC4)
3. Freeverb (Gstreamer)
4. Equalizer Input Gain (Gstreamer volume)
5. Butterworth Highpass filter (Gstreamer audiocheblimit)
6. Butterworth Lowpass filter (Gstreamer audiocheblimit)
7. 15 bands Equalizer (Gstreamer)
8. Equalizer Output Gain (Gstreamer volume)
9. Output Limiter (Ladspa Fast Lookahead Limiter)
10. Spectrum Analyzer (Gstreamer)

Required libraries:

- Python 3
- Python configparser (Python versions higher than 3.5.0 come with it. There is
  no need to install a separate package)
- PyGobject 3
- Python Cairo
- Python Numpy
- Python Scipy (0.18 or above)
- Gtk 3.18 or above
- Gstreamer, Gstreamer Plugins Good, Gstreamer Plugins Bad and Gstreamer Python
 (Since version 1.4.3 Pulseeffects needs Gstreamer 1.12 or above)
- swh-plugins from Ladspa

Arch Linux package:

[https://aur.archlinux.org/packages/pulseeffects/](https://aur.archlinux.org/packages/pulseeffects/)

Note for users trying to install directly from the sources:

The setup.py script only installs the PulseEffects Python module. It does not
copy the files inside the share folder to /usr/share. That is because
python setuptools documentation does not recommends this to be done
through it. The ideal solution would be to have a package for your
distribution. If there is not one available and you would like to try to
install by yourself you can try to manualy copy the files in the share folder
to the corresponding folders inside /usr/share and then run as root
**glib-compile-schemas /usr/share/glib-2.0/schemas/**:

1. git clone https://github.com/wwmm/pulseeffects.git
2. cd pulseeffects
3. sudo python3 setup.py install
4. sudo cp -r share/* /usr/share/
5. sudo glib-compile-schemas /usr/share/glib-2.0/schemas/

In case a package is built for your distribution after you have done the steps
above you will have to remember to remove the files you copied before installing
the package. If you do not want to have this headache a better option is
running PulseEffects without installing it. Do the following steps:

1. git clone https://github.com/wwmm/pulseeffects.git
2. cd pulseeffects
3. glib-compile-schemas share/glib-2.0/schemas/
4. chmod +x pulseeffects
5. GSETTINGS_SCHEMA_DIR=share/glib-2.0/schemas/ ./pulseeffects
