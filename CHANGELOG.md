# Changelog

## [Unreleased]

## [3.1.4]
### Added
- Maximizer plugin from ZamAudio

## [3.1.3]
### Fixed
- Level meters: conversion from decibel to linear scale uses the correct factor
- Auto volume: It is working in service mode and it does not reset the limiter
gain to -10 dB when the main window is opened anymore. It alse makes better
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
