# Changelog

## [Unreleased]
### Fixed
- When the user loaded a preset after upgrading to a PulseEffects version that
had a new plugin (not present in the preset) a crash could happen in a few
situations. I fixed that. Once the user saves the preset its plugin list will
be updated with the new plugin.

## [4.0.5]
### Added
- Expander from Linux Studio Plugins

### Fixed
- Replaced a few C++ shared_ptr by unique_ptr. This should lead to less
memory overhead. The code refactoring that done for this fixed a bug where
sometimes PE did not freed a few resources when closing.
- fixed a bug where PE crashed when the pitch plugin was put before the webrtc

## [4.0.4]
### Fixed
- Fixed a bug introduced in the last update. Applications widget were not being
removed from PE when they were closed

## [4.0.3]
### Added
- improved the algorithm used to enable/disable/reorder plugins.

### Changed
- The default buffer values were increased from 100 ms to 200 ms. Reason: I
noticed that the audio of a few devices like bluetooth headphones have noises
when the buffer value is too low. Users who want to lower the values can do so
in the main settings menu.

### Fixed
- Removed a lot of potential memory leaks. I had a few misconceptions about how
GTK builder is supposed to be used in GTKMM. I did a code refactoring to use
 the api in a safer way.

## [4.0.2]
### Added
- improved the algorithm used to enable/disable/reorder plugins.

### Fixed
- Spectrum widget being shown even when it is disabled
- Fixed excessive level meters readings.
- Mouse motion over spectrum updates the displayed magnitude and frequency.

## [4.0.1]
### Fixed
- No more crashes when the user tries to enable an optional plugin that is not
installed.

## [4.0.0]
### Added
- Users can change effects order. This information is saved in the user preset
like most of the other settings exposed by PulseEffects.
- Users can choose a custom color for the spectrum.
- Users can change the number of equalizer bands in the equalizer menu settings.
This configuration is also saved to the user preset. In the same menu there is
also a facility to calculate the corresponding frequencies of a graphic
 equalizer with the same number of bands. Users with weak processors will
 benefit a lot from this setting as the more bands you have the more cpu is used
- Calf Filter plugin replaces the high pass and low pass filters from previous
PE versions.

### Changed
- From PulseEffects 4.0.0 onwards the presets files uses a completely different
format(we moved from ini to json). This means users will have to create new
presets. I know it sucks but it was necessary.

### Removed
- Calf Stereo Spread
- Per app level meters. They were the source of many bugs that I could never
fix. As it is not a mandatory feature it was best to remove.

### Note for packagers
- PE 4.0.0 marks the move from Python to C++. So Python packages like Numpy
and Scipy are not a dependency anymore. Gstreamer Insertbin is also not used.
C++ Boost is now a dependency. I will leave a python branch on github in case
some bug fix is needed for the 3.x series. But my focus from now on will be
on the 4.x series.

## [3.2.3]
### Fixed
- Fixed input and output limiter presets not being properly loaded

## [3.2.2]
### Added
- GStreamer Webrtc plugin
- Gate and Deesser Plugins from Calf Studio
- Calf Studio compressor replaced the one from swh-plugins
- User can select the input and output device used in the current session. The
selected devices are not remembered when PulseEffects is closed.
- Debug mode: execute PulseEffects doing "PULSEEFFECTS_DEBUG=1 pulseeffects" to
see debug messages.
- Level meter for recording applications
- Buffer and latency of input and output effects can be independently configured
in the settings menu
- Applications buffer and latency values displayed in the main window is updated
every 5 seconds.

### Fixed
- too many decimal places in the frequency and intensity shown in the spectrum
window when hovering the mouse
- Using Gtk.FileChooserNative in the import presets window. Now presets import
will work in sandboxed environments like flatpak
- Level meter streams are not created when running as service with the window
closed. These streams are put in the paused together with the corresponding
application. Both changes should help save cpu.

### Removed Dependencies
- swh-plugins is not a dependency anymore as the limiter and compressor being
used are the ones from Calf Studio Plugins

## [3.2.1]
### Added
- Updated Czech and Italian translations
- Presets can be loaded from command line. Run 'pulseeffects --help' to see the
available options.
- Presets can be imported using the presets menu.

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
