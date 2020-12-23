# PulseEffects Legacy PulseAudio

[![Donate](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/wwmm/donate)

This is the old PulseEffects version supporting plain PulseAudio server, not Pipewire. This branch is in maintenance mode and no longer under development.

![](images/pulseeffects.png)
![](images/equalizer1.png)
![](images/equalizer2.png)
![](images/convolver.png)
![](images/test_signals.png)

## Effects available

### Applications output

- Auto gain
- Bass enhancer
- Compressor
- Convolver
- Crossfeed
- Crystalizer
- De-esser
- Equalizer
- Exciter
- Expander
- Filter (low-pass, high-pass, band-pass and band-reject modes)
- Gate
- Limiter
- Loudness
- Maximizer
- Multiband compressor
- Multiband gate
- Noise reduction
- Pitch
- Reverberation
- Stereo tools

### Applications input

- Compressor
- De-esser
- Equalizer
- Filter (low-pass, high-pass, band-pass and band-reject modes)
- Gate
- Limiter
- Maximizer
- Multiband compressor
- Multiband gate
- Noise reduction
- Pitch
- Reverberation
- Stereo tools
- WebRTC

The user has full control over the effects order. Just use the up/down arrows
next to the plugin label at the left side. To be able to use all plugins you
must have the following installed in your system:

- [PulseAudio](https://gitlab.freedesktop.org/pulseaudio/pulseaudio). Version 13 or higher
- [GStreamer plugins "Good"](https://github.com/GStreamer/gst-plugins-good). Version 1.12.5 or higher
- [GStreamer plugins "Bad"](https://github.com/GStreamer/gst-plugins-bad). Version 1.12.5 or higher
- [Linux Studio plugins](http://lsp-plug.in/?page=home). Version 1.1.24 or higher.
- [Calf Studio plugins](https://calf-studio-gear.org/). Version 0.90.1 or higher.
- [libebur128](https://github.com/jiixyj/libebur128). For Auto Gain.
- [ZamAudio plugins](http://www.zamaudio.com/). For Maximizer.
- [zita-convolver](https://kokkinizita.linuxaudio.org/linuxaudio/). For Convolver.
- [rubberband](https://www.breakfastquay.com/rubberband/). For Pitch Shift.
- [RNNoise](https://github.com/xiph/rnnoise). For Noise Reduction.
- [libsamplerate](http://www.mega-nerd.com/SRC/index.html)
- [libsndfile](http://www.mega-nerd.com/libsndfile/)

## Donate

You can help Wellington Wallace to keep developing PulseEffects through donations.
Any amount will be greatly appreciated :-)

<table>
  <tr>
    <td><a href="https://liberapay.com/wwmm/">https://liberapay.com/wwmm</a></td>
    <td><a href="https://liberapay.com/wwmm/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a></td>
  </tr>
  <tr>
    <td>https://www.patreon.com/wellingtonwallace?fan_landing=true</td>
    <td>Patreon</td>
  </tr>
</table>

## Changelog

- For information about changes between versions take a look at our
  [changelog](https://github.com/wwmm/pulseeffects/blob/master/CHANGELOG.md)

## Installation

See the wiki: [Installing from Source](https://github.com/wwmm/pulseeffects/wiki/Installation-from-Source), for detailed instructions.

## Documentation

PulseEffects documentation can be read inside the GUI installing
[yelp](https://gitlab.gnome.org/GNOME/yelp) package.

## Frequently asked questions

Take a look at our [FAQ](https://github.com/wwmm/pulseeffects/wiki/FAQ) to see
if you are facing a known problem.

## Command-line options

See the wiki: [Command Line Options](https://github.com/wwmm/pulseeffects/wiki/Command-Line-Options)

## Community presets

See the wiki: [Community presets](https://github.com/wwmm/pulseeffects/wiki/Community-presets)

## Reporting bugs

See the wiki: [Reporting Bugs](https://github.com/wwmm/pulseeffects/wiki/Reporting-bugs)

## Translating PulseEffects

See the wiki: [Translating PulseEffects](https://github.com/wwmm/pulseeffects/wiki/Translating-PulseEffects), for detailed instructions.

## License

PulseEffects is licensed under GNU General Public License version 3. See [LICENSE file](https://github.com/wwmm/pulseeffects/blob/master/LICENSE.md).
