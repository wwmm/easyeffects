# PulseEffects

Audio effects for Pulseaudio applications

![](images/pulseeffects.png)
![](images/pulseeffects_equalizer.png)
![](images/pulseeffects_calibration.png)

Order of effects applied to applications output:

1. Input Limiter (Ladspa Fast Lookahead Limiter)
2. Auto Volume
3. Compressor (Ladspa SC4)
4. Butterworth Highpass filter (Gstreamer audiocheblimit)
5. Butterworth Lowpass filter (Gstreamer audiocheblimit)
6. 30 Bands Parametric Equalizer (Gstreamer)
7. Exciter (LV2 Exciter from Calf Studio)
8. Bass Enhancer (LV2 Bass Enhancer from Calf Studio)
9. Delay Compensator(LV2 Delay Compensator from Linux Studio Plugins)
10. Stereo Enhancer (LV2 Stereo Enhancer from Calf Studio)
11. Stereo Spread (LV2 MultiSpread from Calf Studio)
12. Freeverb (Gstreamer)
13. Stereo Panorama (Gstreamer)
14. Maximizer (Ladspa Maximizer from ZamAudio)
15. Output Limiter (Ladspa Fast Lookahead Limiter)
16. Spectrum Analyzer (Gstreamer)

Since version 2.0.0 PulseEffects is capable of applying effects to microphone
output at the same time it applies them for applications output:

1. Input Limiter (Ladspa Fast Lookahead Limiter)
2. Compressor (Ladspa SC4)
3. Butterworth Highpass filter (Gstreamer audiocheblimit)
4. Butterworth Lowpass filter (Gstreamer audiocheblimit)
5. 30 Bands Parametric Equalizer (Gstreamer)
6. Freeverb (Gstreamer)
7. Spectrum Analyzer (Gstreamer)

## Installation

### GNU/Linux Packages

- [Arch Linux](https://aur.archlinux.org/packages/pulseeffects/)
- [Void Linux](https://github.com/voidlinux/void-packages/blob/master/srcpkgs/pulseeffects/template)
- [Gentoo](https://packages.gentoo.org/packages/media-sound/pulseeffects/)

#### Community Packages

These are community maintained repositories of distribution packages. You can
find more information about these in the
[wiki](https://github.com/wwmm/pulseeffects/wiki/Package-Repositories#package-repositories).

- [Ubuntu](https://github.com/wwmm/pulseeffects/wiki/Package-Repositories#ubuntu-1710-and-newer)

### Flatpak

[Flatpak](https://flatpak.org) packages support multiple distributions and are sandboxed.

Stable releases are hosted on [Flathub](https://flathub.org):

```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub com.github.wwmm.pulseeffects
```

### Source Code

Required libraries:

- Python 3
- Python configparser (Included with Python3 > 3.5.0. There is
  no need to install it.)
- [PyGObject](https://pygobject.readthedocs.io/en/latest/)
- [Python Cairo](https://cairographics.org/pycairo/)
- [Python Numpy](http://www.numpy.org/)
- [Python Scipy](https://scipy.org/scipylib/) (0.18 or above)
- Gtk 3.18 or above
- Gstreamer, Gstreamer Plugins Good, Gstreamer Plugins Bad and Gstreamer Python
 (Since version 1.4.3 Pulseeffects needs Gstreamer 1.12 or above)
- [swh-plugins](https://github.com/swh/ladspa) from Ladspa
- [Lilv](http://drobilla.net/category/lilv/)
- [Calf Plugins](https://calf-studio-gear.org/)
- [ZamAudio Ladspa Plugins](http://www.zamaudio.com/)
- [Linux Studio Plugins](http://lsp-plug.in/)

#### Installing from Source

See the wiki: [Installing from Source](https://github.com/wwmm/pulseeffects/wiki/Installation-from-Source), for detailed instructions.

## Command Line Options

See the wiki: [Command Line Options](https://github.com/wwmm/pulseeffects/wiki/Command-Line-Options)

## Reporting bugs

See the wiki: [Reporting Bugs](https://github.com/wwmm/pulseeffects/wiki/Reporting-bugs)

## Translating PulseEffects

See the wiki: [Translating PulseEffects](https://github.com/wwmm/pulseeffects/wiki/Translating-PulseEffects), for detailed instructions.
