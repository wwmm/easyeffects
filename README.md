# PulseEffects

Audio effects for Pulseaudio applications

![](images/pulseeffects.png)
![](images/equalizer1.png)
![](images/equalizer2.png)
![](images/test_signals.png)

## Effects Available

### Applications Output

- Limiter
- Expander
- Compressor
- Multiband Compressor
- Equalizer
- Bass Enhancer
- Exciter
- Reverberation
- Delay
- Crossfeed
- Filter (Lowpass, Highpass, Bandpass and Bandreject modes)
- Stereo Enhancer
- Loudness
- Maximizer

### Applications input

- Gate
- Webrtc
- Limiter
- Compressor
- Multiband Compressor
- Equalizer
- Reverberation
- Pitch
- Filter (Lowpass, Highpass, Bandpass and Bandreject modes)
- Deesser

The user has full control over the effects order. Just use the up/down arrows
next to the plugin label at the left side. To be able to use all plugins you
 must have the following installed in your system:

- [GStreamer Plugins Good](https://github.com/GStreamer/gst-plugins-good).
Version `1.12.5` or higher
- [GStreamer Plugins Bad](https://github.com/GStreamer/gst-plugins-bad).
Version `1.12.5` or higher
- [Calf Studio Plugins](https://calf-studio-gear.org/). Version `0.90` or higher.
- [Linux Studio Plugins](http://lsp-plug.in/)
- [ZamAudio Plugins](http://www.zamaudio.com/)
- [MDA.LV2](https://git.drobilla.net/cgit.cgi/mda.lv2.git/about/)

## Installation

### GNU/Linux Packages

- [Arch Linux](https://aur.archlinux.org/packages/pulseeffects/)
- [Void Linux](https://github.com/voidlinux/void-packages/blob/master/srcpkgs/pulseeffects/template)
- [Gentoo](https://packages.gentoo.org/packages/media-sound/pulseeffects/)
- [ROSA](https://abf.io/import/pulseeffects/)
- [ALT Linux](https://packages.altlinux.org/Sisyphus/srpms/pulseeffects/)

#### Community Packages

These are community maintained repositories of distribution packages. You can
find more information about these in the
[wiki](https://github.com/wwmm/pulseeffects/wiki/Package-Repositories#package-repositories).

- [Ubuntu and Debian](https://github.com/wwmm/pulseeffects/wiki/Package-Repositories#debian--ubuntu)

### Flatpak

[Flatpak](https://flatpak.org) packages support multiple distributions and are sandboxed.

Stable releases are hosted on [Flathub](https://flathub.org):

```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub com.github.wwmm.pulseeffects
```

#### Installing from Source

See the wiki: [Installing from Source](https://github.com/wwmm/pulseeffects/wiki/Installation-from-Source), for detailed instructions.

## Frequently asked Questions

Take a look at our [FAQ](https://github.com/wwmm/pulseeffects/wiki/FAQ) to see
if you are facing a known problem.

## Command Line Options

See the wiki: [Command Line Options](https://github.com/wwmm/pulseeffects/wiki/Command-Line-Options)

## Community presets

See the wiki: [Community presets](https://github.com/wwmm/pulseeffects/wiki/Community-presets)

## Reporting bugs

See the wiki: [Reporting Bugs](https://github.com/wwmm/pulseeffects/wiki/Reporting-bugs)

## Translating PulseEffects

See the wiki: [Translating PulseEffects](https://github.com/wwmm/pulseeffects/wiki/Translating-PulseEffects), for detailed instructions.
