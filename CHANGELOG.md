# Changelog

## [Unreleased]
### Added - Updated Czech translation

## [3.2.0]
### Added
- Crossfeed plugin from bs2b library. GStreamer Plugins Bad must be compiled
against it.
- PulseEffects pipeline enters the playing state only when there is at least
one audio application switched on and in the playing state. This will save cpu
when the user paused or switched off all audio applications while PulseEffects
is running
- Log output is a little more clear

### Fixed
- Do no try to read delay plugin properties when it is not available. This will
flood the system log with error messages

## [3.1.9]
### Fixed
- Fixed exciter ceiling parameter not being loaded from the presets file

## [3.1.8]
### Fixed
- Fixed a critical bug where plug and unplugging a usb microphone a few times
would break PulseEffects routing.

## [3.1.7]
### Added
- Delay Compensator plugin from Linux Studio Plugins (LV2 version)
- Pitch Shifting plugin from Rubber Band library (Ladspa)
- New settings menu.

### Fixed
- Do not force the "switch all apps" setting to true when running as service.
- Fixed frequency value in band label being slightly different from the one
in the frequency menu because of rounding problems
- Removed unnecessary spaces between a few elements in the interface
- Test signals application automatically switches to the default microphone
- Equal loudness test signals frequencies match the equalizer default
frequencies

## [3.1.6]
### Added
- MultiSpread plugin from Calf Studio

## [3.1.5]
### Added
- Stereo Enhancer from Calf Studio

### Fixed
- Check marks are not shown for plugins that are not available
- Fixed a bug were the volume meter was not updated in applications like the
game XCOM

## [3.1.4]
### Added
- Maximizer plugin from ZamAudio

## [3.1.3]
### Fixed
- Level meters: conversion from decibel to linear scale uses the correct factor
- Auto volume: It is working in service mode and it does not reset the limiter
gain to -10 dB when the main window is opened anymore. It also makes better
use of the limiter attenuation parameter and reduces its input gain whenever
 there is attenuation in the limiter.
- Interface: Uses a little less screen space.

## [3.1.2]
### Fixed
- Fixed a bug where the pipeline could be put out of the playing state when
running in service mode.
- We do not emit level meter and spectrum messages when running in service mode
anymore. This should improve service performance on weak CPUs.

## [3.1.1]
### Fixed
- Missing Calf plugins do not crash PulseEffects anymore. They are optional.

## [3.1.0]
### Added
- Meson: gstreamer-insetbin dependency check
- New plugins: Bass Enhancer and Exciter LV2 plugins from Calf Studio
- Interface: The enable/disable switch was moved from the plugin selection list
to the  respective plugin control section. Let's hope this makes things easier
new users. At the previous location of the switch users will find a check
informing if the plugin is enabled or not.
- Autostart service: Added an option in the main menu to autostart pulseeffects
in service mode (--gapplication-service) at login. under the hood what is done
is copying a .desktop file to the autostart folder in the user config folder
(usually ~/.config).

### Fixed
Enable all apps settings: it is not reset to true when closing the window
The following translations were updated: cs, pt_BR

## [3.0.9]
### Added
- Equalizer: it now has 30 bands fully configurable by the user
- Global: enable all apps switch added to the main menu
- App info: apps on/off switch is now near the apps name and icon. Let's hope
that this makes things more obvious to new users.
