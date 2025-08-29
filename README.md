<h2 align="center">
  <img src="src/contents/icons/com.github.wwmm.easyeffects.svg" alt="Easy Effects icon" width="150" height="150"/>
  <br>
  Easy Effects
</h2>

<p align="center">
  <strong>Audio effects for PipeWire applications</strong>
</p>

<p align="center">
  <a href="https://github.com/sponsors/wwmm">
    <img alt="GitHub Sponsors donation button" src="https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&link=https://github.com/sponsors/wwmm">
  </a>
  <a href="https://liberapay.com/wwmm/donate">
    <img alt="Liberapay donation button" src="https://img.shields.io/badge/liberapay-donate-green">
  </a>
  <a href="https://www.patreon.com/wellingtonwallace?fan_landing=true">
    <img alt="Patreon donation button" src="https://img.shields.io/badge/patreon-donate-green.svg">
  </a>
  <a href="https://www.paypal.com/donate?hosted_button_id=RK723F4EKH2UE">
    <img alt="PayPal donation button" src="https://img.shields.io/badge/paypal-donate-green.svg">
  </a>
</p>

<br>

<p align="center">
  <a href="https://hosted.weblate.org/engage/easyeffects/">
    <img alt="Translation status" src="https://hosted.weblate.org/widgets/easyeffects/-/svg-badge.svg"/>
  </a>
  <a href="https://github.com/wwmm/easyeffects/actions/workflows/CI.yaml">
    <img alt="CI status" src="https://github.com/wwmm/easyeffects/actions/workflows/CI.yaml/badge.svg"/>
  </a>
  <a href="https://circleci.com/gh/wwmm/easyeffects">
    <img alt="CircleCI status" src="https://circleci.com/gh/wwmm/easyeffects.svg?style=shield"/>
  </a>
</p>

This application was formerly known as PulseEffects, but it was renamed to Easy Effects after it started to use GTK4 and
GStreamer usage was replaced by native PipeWire filters. And eventually the whole application was ported from gtk4 to
a combination of Qt, QML and KDe/Kirigami.

![GitHub Light](images/easyeffects-light-screenshot-1.png#gh-light-mode-only)
![GitHub Light](images/easyeffects-light-screenshot-2.png#gh-light-mode-only)
![GitHub Light](images/easyeffects-light-screenshot-3.png#gh-light-mode-only)

![GitHub Dark](images/easyeffects-dark-screenshot-1.png#gh-dark-mode-only)
![GitHub Dark](images/easyeffects-dark-screenshot-2.png#gh-dark-mode-only)
![GitHub Dark](images/easyeffects-dark-screenshot-3.png#gh-dark-mode-only)

## Effects available

- Auto gain
- Bass enhancer
- Bass loudness
- Compressor
- Convolver
- Crossfeed
- Crystalizer
- De-esser
- Delay
- Deep noise remover
- Echo canceller
- Equalizer
- Exciter
- Expander
- Filter (low-pass, high-pass, band-pass and band-reject modes)
- Gate
- Level meter
- Limiter
- Loudness
- Maximizer
- Multiband compressor
- Multiband gate
- Noise reduction
- Pitch shift
- Reverberation
- Speech processor
- Stereo tools

The user has full control over the effects order. Just use the up/down arrows
next to the effect labels on the left side.

Some packages do not provide all plugin packages by default. In case some effects are not available, ensure you have the following installed on your system:

<details>
<summary>Dependencies</summary>

Plugins needed for effects:

- [Linux Studio plugins](https://lsp-plug.in/). Version 1.1.24 or higher.
- [Calf Studio plugins](https://calf-studio-gear.org/). Version 0.90.1 or higher.
- [Libebur128](https://github.com/jiixyj/libebur128). For Auto gain and Level meter.
- [ZamAudio plugins](https://www.zamaudio.com/). For Maximizer.
- [Zita-convolver](https://kokkinizita.linuxaudio.org/linuxaudio/). For Convolver.
- [MDA](https://gitlab.com/drobilla/mda-lv2). For Bass loudness.
- [SpeexDSP](https://www.speex.org/). For Speech processor.
- [SoundTouch](https://www.surina.net/soundtouch/). For Pitch shift.
- [RNNoise](https://gitlab.xiph.org/xiph/rnnoise). For Noise reduction.
- [DeepFilterNet](https://github.com/Rikorose/DeepFilterNet). For Deep noise remover.

Other dependencies include:
- [libsamplerate](http://www.mega-nerd.com/SRC/index.html)
- [libsndfile](http://www.mega-nerd.com/libsndfile/)
- [libbs2b](https://sourceforge.net/projects/bs2b/files/libbs2b/)
- [fftw](https://fftw.org/)
- [speexdsp](https://www.speex.org/)
- [nlohmann json](https://github.com/nlohmann/json)
- [tbb](https://www.threadingbuildingblocks.org)

</details>

## Donate

You can help me to keep developing Easy Effects through donations. Any amount will be greatly appreciated :-)

[GitHub Sponsors](https://github.com/sponsors/wwmm)  
[Liberapay](https://liberapay.com/wwmm)  
[Patreon](https://www.patreon.com/wellingtonwallace?fan_landing=true)  
[PayPal](https://www.paypal.com/donate?hosted_button_id=RK723F4EKH2UE)  

## Installation

### Distribution-specific packages

Most up-to-date Linux distributions have an `easyeffects` package that can be installed with the distribution package manager. See [the wiki](https://github.com/wwmm/easyeffects/wiki/Package-Repositories) for a full list.

### Flatpak

Alternatively, click here to install via Flatpak:

<a href='https://flathub.org/apps/details/com.github.wwmm.easyeffects'><img width='240' alt='Download Easy Effects on Flathub' src='https://flathub.org/assets/badges/flathub-badge-en.png'/></a>

Flatpak packages support most Linux distributions and are sandboxed. The Easy Effects Flatpak package also includes all available plugins.

If your distribution does not yet include packages required to build Easy Effects, Flatpak is a convenient option.

## Warning

**Do not set** Easy Effects virtual devices as your default audio input/output.
Easy Effects is designed assuming that your hardware stays as default device.

## Help

A comprehensive set of help pages are included in the application itself, accessed via the hamburger menu in the top right. If the help pages are inaccessible ensure the [yelp](https://gitlab.gnome.org/GNOME/yelp) package is installed.

The latest version of the help pages can also be [seen here](https://wwmm.github.io/easyeffects).

## Changelog

For information about changes between versions, take a look at our [changelog](https://github.com/wwmm/easyeffects/blob/master/CHANGELOG.md)

## Frequently asked questions

Take a look at our [FAQ](https://github.com/wwmm/easyeffects/wiki/FAQ) to see
if you are facing a known issue.

## Community presets

See the wiki: [Community presets](https://github.com/wwmm/easyeffects/wiki/Community-presets)

This page also has instructions for converting PulseEffects presets to Easy Effects presets.

Guildelines for package maintainers: [Guidelines](COMMUNITY_PRESETS_GUIDELINES.md)

## Command-line options

See the wiki: [Command Line Options](https://github.com/wwmm/easyeffects/wiki/Command-Line-Options)

## GNOME Shell extension

See the wiki: [Preset Selector GNOME Extension](https://github.com/wwmm/easyeffects/wiki/Preset-Selector-GNOME-Extension)

## Reporting bugs

See the wiki: [Reporting Bugs](https://github.com/wwmm/easyeffects/wiki/Reporting-bugs)

## Translating Easy Effects

See the wiki's [Translating Easy Effects](https://github.com/wwmm/easyeffects/wiki/Translating-EasyEffects) article for detailed instructions.

## Installing from source

To install from source, see the wiki's [Installing from Source](https://github.com/wwmm/easyeffects/wiki/Installation-from-Source) article for detailed instructions.

## CI artifacts

Flatpak bundles and Arch packages for testing and development purposes are built for PRs and pushes to master.
See the [installation instructions](https://github.com/wwmm/easyeffects/wiki/Package-Repositories#ci-artifacts).

## License

Easy Effects is licensed under GNU General Public License version 3 or later. See the [LICENSE file](https://github.com/wwmm/easyeffects/blob/master/LICENSE).
