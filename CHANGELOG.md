# Changelog

## [Unreleased]
### Added
- A convolver plugin. It is the first plugin I have written :D :-) As audio
processing is not my original field of expertise I prefer to use plugins from
mature projects like Calf. But GStreamer support for LV2 plugins is limited and
none of the available convolvers work in it. You can see more details here
https://github.com/wwmm/pulseeffects/issues/164. As this situation will not
change any time soon I had to put my hands in the dirty. The plugin is based on
zita-convolver library and while writing it I took inspiration in the work of
other people you can find here https://github.com/tomszilagyi/ir.lv2/ and here
 https://github.com/x42/convoLV2. For now only stereo impulse response files
encoded as wav are supported.
- Import presets dialog now has filters to show only presets files (.json).
- The impulse response file import dialog also has a filter (.irs). In case your
response file is *.wav just rename it to *.irs.
- User can change Pulseaudio blocksize. This can help him/her to fine tune the
latency and in case the convolver is being used its cpu usage.

### Note for packagers
- There is no need to add the zita-convolver library as dependency. Its source
is shipped with PulseEffects and compiled with PulseEffects. This means that
the library fftw3f that is a Zita dependency is now also a PulseEffects
dependency.

## [4.1.3]
### Fixed
- The equalizer presets were not being applied in a few situations

## [4.1.2]
### Added
- Multiband gate plugin from Calf Studio
- Stereo Tools plugin from Calf Studio
- Presets menu label shows the last selected preset when PulseEffects is
restarted.
- The Deesser can be used in both pipelines.
- The so called "perfect eq" preset. Not that there is something we can call
a perfect equalizer preset. But people like to use it as a starting point.So I
added it.

### Fixed
- Deesser "listen" control was not working.  

### Removed
- Expander plugin. Reason: I thought it was open source but it is proprietary.
 This complicate packagers life and philosophically I prefer to use only
 open source plugins. As Calf Gate can be used as an expander it now replaces
 the plugin from Linux Studio.
- Delay plugin. Although its source is distributed its license has very annoying
restriction that prohibits packagers to apply patches without the plugin author
permission. As Calf Stereo Tools can be used to apply delays between the left
and the right channels it now replaces the delay plugin from Linux Studio.
- Panorama plugin. Its functionality is also offered by Calf Stereo Tools.

### Packagers
- For all the reasons explained above Linux Studio Plugins is not a dependency
anymore and can be safely removed from the build scripts.

## [4.1.1]
### Added
- Added loudness plugin from MDA.LV2

## [4.1.0]
- Fixed multiband compressor schema not being installed.

## [4.0.9]
### Added
- GStreamer 10 bands equalizer presets: rock, soft, pop, etc.
- Input gain and output gain controls for the equalizer.
- Added Calf Multiband compressor

### Fixed
- Compressor and gate plugins knee parameter was always at its default value.
 Fixed that.
 - Removed a small memory leak(nothing critical).

### Information for packagers
- The limiter plugin in latest Calf Studio release (0.90) has a memory leak.
I reported it to Calf developers and it was fixed. See
https://github.com/calf-studio-gear/calf/issues/180. But as it may take sometime
till the next Calf is released I suggest that everybody using the limiter plugin
in PulseEffects either install a Calf package built from git master or backport
the bug fix (3 lines) to 0.90 somehow. For most users it is not a critical leak.
But people that do not turn off their computers everyday should be concerned as
the leak is triggered whenever PulseEffects alternates between the playing
 and paused state.

## [4.0.8]
### Fixed
- Removed more memory leaks(nothing critical)
- The presets menu list is scrollable again.

## [4.0.7]
### Fixed
- Fixed Limit Parameter of the Limiter plugin being reset to -11 dB whenever a
preset was loaded. Now the user value saved in the preset is correctly loaded

## [4.0.6]
### Added
- The user can change the spectrum height
- More parameters are saved to the user preset: buffer, latency and spectrum
parameters. It is particularly useful to have the buffer value saved to the
preset because a few devices like bluetooth headphones need values that are
very different than the ones used in soundcards.

### Fixed
- When the user loaded a preset after upgrading to a PulseEffects version that
had a new plugin (not present in the preset) a crash could happen in a few
situations. I fixed that. Once the user saves the preset its plugin list will
be updated with the new plugin.
- Sometimes when the user opened PE window while running in service mode the
spectrum was not enabled. I fixed that.

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
