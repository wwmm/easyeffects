# EasyEffects

[![CircleCI](https://circleci.com/gh/wwmm/easyeffects.svg?style=shield)](https://circleci.com/gh/wwmm/easyeffects)
[![Donate](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/wwmm/donate)
[![Translation Status](https://hosted.weblate.org/widgets/easyeffects/-/287x66-grey.png)](https://hosted.weblate.org/engage/easyeffects/)

Audio effects for PipeWire applications.

This application was formerly known as PulseEffects, but it was renamed to EasyEffects after it started to use GTK4 and
GStreamer usage was replaced by native PipeWire filters.

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
- Echo Canceller
- Equalizer
- Exciter
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

The user has full control over the effects order. Just use the up/down arrows
next to the effect labels on the left side.

To be able to use all of the effects, you must have the following installed on your system:

- [Linux Studio plugins](http://lsp-plug.in/?page=home). Version 1.1.24 or higher.
- [Calf Studio plugins](https://calf-studio-gear.org/). Version 0.90.1 or higher.
- [libebur128](https://github.com/jiixyj/libebur128). For Auto Gain.
- [ZamAudio plugins](http://www.zamaudio.com/). For Maximizer.
- [zita-convolver](https://kokkinizita.linuxaudio.org/linuxaudio/). For Convolver.
- [rubberband](https://www.breakfastquay.com/rubberband/). For Pitch Shift.
- [RNNoise](https://github.com/xiph/rnnoise). For Noise Reduction.
- [libsamplerate](http://www.mega-nerd.com/SRC/index.html)
- [libsndfile](http://www.mega-nerd.com/libsndfile/)
- [libbs2b](https://sourceforge.net/projects/bs2b/files/libbs2b/)
- [fftw](https://fftw.org/)
- [speexdsp](https://www.speex.org/)
- [nlohmann json](https://github.com/nlohmann/json)
- [tbb](https://www.threadingbuildingblocks.org)

## Donate

You can help me to keep developing EasyEffects through donations. Any amount will be greatly appreciated :-)

<table>
  <tr>
    <td>https://github.com/sponsors/wwmm</td>
    <td>Github Sponsors</td>
  </tr>
  <tr>
    <td><a href="https://liberapay.com/wwmm/">https://liberapay.com/wwmm</a></td>
    <td><a href="https://liberapay.com/wwmm/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a></td>
  </tr>
  <tr>
    <td>https://www.patreon.com/wellingtonwallace?fan_landing=true</td>
    <td>Patreon</td>
  </tr>
  <tr>
    <td>https://www.paypal.com/donate?hosted_button_id=RK723F4EKH2UE</td>
    <td>PayPal</td>
  </tr>
</table>

## Changelog

- For information about changes between versions, take a look at our
  [changelog](https://github.com/wwmm/easyeffects/blob/master/CHANGELOG.md)
  
## Warning
DON'T SET EasyEffects virtual devices as default
EasyEffects is designed assuming that your hardware stays as default

## Installation

### Distribution-specific packages

These Linux distributions have EasyEffects packages:

- [Arch Linux](https://www.archlinux.org/packages/community/x86_64/easyeffects/) / [AUR Git version](https://aur.archlinux.org/packages/easyeffects-git/)
- [openSUSE](https://software.opensuse.org/package/easyeffects)
- [NixOS](https://search.nixos.org/packages?channel=unstable&show=easyeffects&query=easyeffects)
- [Gentoo](https://packages.gentoo.org/packages/media-sound/easyeffects)
- [Fedora](https://src.fedoraproject.org/rpms/easyeffects)

<!--
- [Void Linux](https://github.com/void-linux/void-packages/blob/master/srcpkgs/easyeffects/template)
- [ROSA](https://abf.io/import/easyeffects/)
- [ALT Linux](https://packages.altlinux.org/Sisyphus/srpms/easyeffects/)
- [Fedora GNU/Linux](https://apps.fedoraproject.org/packages/easyeffects)
- [CRUX](https://crux.nu/portdb/?a=search&q=easyeffects)
- [Debian (buster)](https://packages.debian.org/buster-backports/easyeffects) / [Debian (bullseye)](https://packages.debian.org/bullseye/easyeffects) -->

<!-- These are community maintained repositories of distribution packages. You can
find more information about these in the
[wiki](https://github.com/wwmm/easyeffects/wiki/Package-Repositories#package-repositories).

- [Ubuntu and Debian](https://github.com/wwmm/easyeffects/wiki/Package-Repositories#debian--ubuntu) -->

### Flatpak

Alternatively, click here to install via Flatpak:

<a href='https://flathub.org/apps/details/com.github.wwmm.easyeffects'><img width='240' alt='Download EasyEffects on Flathub' src='https://flathub.org/assets/badges/flathub-badge-en.png'/></a>

Flatpak packages support most Linux distributions and are sandboxed. The EasyEffects Flatpak package also includes all available plugins.

If your distribution does not yet include packages required to build EasyEffects, Flatpak is a convenient option.

#### Nightly Flatpak

A nightly Flatpak package is also available, to provide the latest changes. See the [installation instructions](https://github.com/wwmm/easyeffects/wiki/Package-Repositories#nightly-flatpak).

### Installing from source

To install from source, see the wiki's [Installing from Source](https://github.com/wwmm/easyeffects/wiki/Installation-from-Source) article for detailed instructions.

## Documentation

EasyEffects documentation can be read inside the GUI if the
[yelp](https://gitlab.gnome.org/GNOME/yelp) package is installed.

## Frequently asked questions

Take a look at our [FAQ](https://github.com/wwmm/easyeffects/wiki/FAQ) to see
if you are facing a known issue.

## Command-line options

See the wiki: [Command Line Options](https://github.com/wwmm/easyeffects/wiki/Command-Line-Options)

## Community presets

See the wiki: [Community presets](https://github.com/wwmm/easyeffects/wiki/Community-presets)

This page also has instructions for converting PulseEffects presets to EasyEffects presets.

## Reporting bugs

See the wiki: [Reporting Bugs](https://github.com/wwmm/easyeffects/wiki/Reporting-bugs)

## Translating EasyEffects

See the wiki's [Translating EasyEffects](https://github.com/wwmm/easyeffects/wiki/Translating-EasyEffects) article for detailed instructions.

## License

EasyEffects is licensed under GNU General Public License version 3. See the [LICENSE file](https://github.com/wwmm/easyeffects/blob/master/LICENSE.md).
