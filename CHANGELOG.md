# Changelog
 
## 7.0.0
### 2022-11-02


### Features:
- It is now possible to add more than one filter instance to the effects pipeline.
- A new `Speech Processor` plugin based on the Speex library was added. Besides providing noise suppression it also can detect voice activity. It is a decent alternative to the cases whre the rnnoise library does not work well.
- Improved compatibility with Linux Studio Plugins `1.2.3`.
- GraphicEQ presets can be imported into the Equalizer effect.
- The application name has been changed to `Easy Effects` to adhere to the naming conventions of GNOME Human Interface Guidelines.
- The documentation has been updated.
- Improved presets autoloading.
- The Autogain silence threshold can now be configured by the user.
- Dry and wet controls added to the Stereo Tools effect.
- The echo canceller now has a residual noise suppression control also based on the Speex library.

### Bug fixes:
- When the mouse was hovering over a chart the wrong value for the x axis coordinate was being shown. This regression is fixed.

### Other notes:
- Because of the new multiple instances feature, the preset format has changed, but the old one is still compatible to be loaded. Anyway take in consideration that saving the current preset will always write it in the new format.

 
## 6.3.0
### 2022-09-02


### Features:
- Multiband Gate implementation has been migrated from CALF to Linux Studio Plugins.
- The preset autoloading code compatibility with bluez5 devices has been improved.
- Wet/dry controls were added for some plugins
- Effect interface is no longer loaded when the related lv2 plugin is not installed on the system. In its place a status message to the user is shown.
- The documentation has been updated
- Improved debug messages
- Updated translations

### Bug fixes:

### Other notes:

 
## 6.2.8
### 2022-07-25


### Features:
- Improved translations

### Bug fixes:
- Fixed a bug where EasyEffects could crash when closing its window while effects were being applied.

### Other notes:

 
## 6.2.7
### 2022-07-18


### Features:
- The bypass state can be saved to the preset file. The reasons why this was done can be seen at https://github.com/wwmm/easyeffects/issues/1039
- The preset autoloading code compatibility with usb devices has been improved.
- A dialog is shown to the user when a preset fails to load or a preset/impulse file fails to be imported.
- The SideChain Gate plugin from `Linux Studio Plugins` is now used instead of the one from Calf Studio.

### Bug fixes:
- EasyEffects should not crash anymore when the user locale is not properly configured.
- A workaround was implemented in our icon to deal with the lack of proper SVG support in QT.

### Other notes:

 
## 6.2.6
### 2022-06-22


### Features:
- When effects are disable to an application we now set its target metadata to null. This will allow the media session
 manager (wireplumber) to properly move the stream to a new device.
- A new configuration option was added. It allows EasyEffects to ignore streams whose purpose is to monitor sink
 devices. This will help to fix some of problems our users were having when using OBS.
- The code that shows the stream sample format has been improved
- The rnnoise library is now optional. This should help package maintainers to build
 a Debian package. See https://github.com/wwmm/easyeffects/issues/1000 for more information.
- Our logs now show the source code line where the messages are being printed
### Bug fixes:
- The "enable effects" checkbox in our window was not being updated when third party programs like pavucontrol
 moved the stream away from our virtual devices. This should be fixed now.
- Fixed a crash that could happen when the maximum autogain history was changed.
- Avoid crashes when pw-mididump is running

 
## 6.2.5
### 2022-04-30


### Features:
- The interface of the pitch plugin was improved
- Our application icon is now compatible with desktops that uses QT
- Our blocklist code will use the `application.id` tag if the stream sets it
- In order to avoid problems with the mouse scroll the entries in the applications list shown in our `Players/Recorders` tab do not show a volume scale anymore. More details about the problem and the solution for it can be found on https://github.com/wwmm/easyeffects/issues/1211 and https://github.com/wwmm/easyeffects/issues/1427
- When no application is available for display in the `Players/Recorders` a message will be shown to the user
- Many translation updates
### Bug fixes:
- Fixed a bug where EasyEffeects crashed when the number of points displayed in the spectrum was changed while our pipeline was active and the spectrum widget was visible
- The pipeline latency value displayed in our window could be wrong in some situations. This was fixed.

 
## 6.2.4
### 2022-03-07


### Features:
- There is a new setting allowing the user to select an inactivity timeout for the pipeline. When no client is playing to or recording from our devices the filters will be unlinked after the timeout is reached. This is done to make sure we do not waste CPU power processing silence.
- The autogain plugin now allows the user to select which of the three loudness are used to calculate the geometric mean.
- The autogain plugin now allows the maximum history to be set and does not use `libebur128` histogram mode anymore. This should avoid the cases where the `Integrated` loudness gets stuck forever in the same value.
- EasyEffects icon has been updated in a way that should make it visible in QT desktops.
### Bug fixes:
- The command line option that returns the global bypass state is working again.

 
## 6.2.3
### 2022-01-29


### Features:
- The crossfeed filter should deal better with PipeWire's dynamic latency switches. Jumps in volume level should not happen anymore in these situations.

### Bug fixes:

- Fixed a bug that prevented mono microphones from properly working with EasyEffects

 
## 6.2.2
### 2022-01-27


### Features:
- Support for the next PipeWire release `0.3.44`
- The autogain filter should deal better with PipeWire's dynamic latency switches. Jumps in volume level should not happen anymore in these situations.
- We added an option that allows the volume and mute state of our virtual devices to be reset when EasyEffects starts. It should help with the cases were our devices are muted by the audio server for unknown reasons.
- Better support for computer suspending.
- Updated translations

### Bug fixes:

- Fixed a bug where trying to create an autoloading profile without having presets caused the application to crash.

 
## 6.2.1
### 2022-01-05


### Bug fixes:
- Fixed a bug where setting a equalizer band `quality` to `zero` would lead to an application crash.

 
## 6.2.0
### 2022-01-04


### Features:
- LibAdwaita is used to create some parts of our window and for handling the switching between dark and light themes.
- The settings menu has been redesigned using LibAdwaita widgets.
- Equalizer APO preset import feature has been improved to apply not only the Bell filter, but also other supported ones (at the moment only the Bandpass filter is not available in LSP plugin).
- The `Reset All Settings` function in our menu should work in Flatpak now.
- We have a new option that allows the user to disable our menus `autohide`. This may help to workaround some bugs Popover menus currently have on gtk4.

### Bug fixes:

- More robust parsing to import APO presets saved with comma as thousands separator in central frequency band.

### Other notes:

- The fmt library is a new dependency https://github.com/fmtlib/fmt. At least while the c++ compilers do not implement its features. This is expected to happen in the next years.
- GTKMM and GLIBMM are not a dependency anymore. We now use gtk4 directly.

 
## 6.1.5
### 2021-11-17


### Features:
- It is now possible to combine impulse responses in the Convolver interface. A new impulse file is generated and it should be visible in the impulse list.
- Improved `x axis` drawings in our plots. Now the number of labels is adjusted dynamically depending on our window width.
- The documentation has been updated reflecting the new EasyEffects features. Old references about PulseEffects have been removed. The documentation button has been added in the menu section.

### Bug fixes:

- When a spinbutton is filled with an out of range value, now it is updated with the lowest/highest value rather than resetting to the previous one.
- The application window now remembers the maximized state and restores it on the next opening event.

### Other notes:

- The `tbb` library is a new dependency https://www.threadingbuildingblocks.org

 
## 6.1.4
### 2021-10-16


### Features:
- The Limiter and the Multiband Compressor plugins can now use an optional external sidechain.
- The Autogain plugin now allows the user to select which Loudness is used as reference for the volume correction.
- The APO Profile Import feature of Equalizer plugin now parses the "Pre Amplification" parameter.
- Optional Cubic Volume can be enabled in General Settings.

### Bug fixes:

- The Spectrum plugin was supposed to enter passthrough whenever it was not visible, but this was happening only when our window was closed.
- Improved support for Assistive Technology.
- The probes used in some filters like the Compressor and the Limiter were not being relinked after changing the order of the plugins in the pipeline. It should be working now.

 
## 6.1.3
### 2021-10-03


### Features:
- PipeWire monitor streams are now excluded and removed from the applications list.

### Bug fixes:

- Hopefully crashes like the one reported at https://github.com/wwmm/easyeffects/issues/1172 are fixed.
- Prevented a case in which Spectrum was crashing.
- Pavucontrol is not added anymore to input applications list on systems with localization different than English.

 
## 6.1.2
### 2021-09-20


### Features:
- Improved compatibility with WirePlumber. This is needed to run on systems that decided to use it instead of the built-in PipeWire session manager. More information at https://github.com/wwmm/easyeffects/issues/1144.

 
## 6.1.1
### 2021-09-17


### Features:
- When trying to add an autoloading profile for a device already in the list its target preset will be updated. This way we can change the profile preset without having to remove and recreating it.
- The preset autoloading support implementation was redesigned again. It should work on more hardware now. For more information see https://github.com/wwmm/easyeffects/issues/1051.
- If the Limiter or the Maximizer are set in the last position of the plugin stack, new plugins are added at the second to last position in order to prevent hardware damage on eventually high output level.
- Removing an application from the blocklist, its previous enabled state is restored.

### Bug fixes:

- Sometimes when removing imported models from the noise reduction plugin the current used model was not properly updated. This should be fixed now.
- When editing presets files in an external editor, duplicated entries won't be shown in our presets menu.
- Now the blocklist is correctly set when switching presets.
- Now the status of the global bypass button is correctly updated when changing plugin stack.
- Missing icons on the system should not be shown inside the application info UI (if an application icon could not be shown even if you're sure it's correctly installed, please open an issue).
- Some icons not showing in Plasma DE with Breeze icon theme should appear now.

 
## 6.1.0
### 2021-08-17


### Features:
- Updated Chinese translation.
- Updated Italian translation.
- Added support for the compressor parameter `Boost Amount`
- The multiband compressor plugin now uses the stereo multiband compressor plugin from Linux Studio Plugins instead of the one from Calf Studio.
- The limiter plugin now uses the stereo limiter plugin from Linux Studio Plugins instead of the one from Calf Studio.
- LV2 filters now are spawned in PipeWire graph only when loaded the first time. Once loaded, they remain connected until EasyEffects shutdown.

### Bug fixes:

- The echo canceller sampling rate is now properly set.
- The threshold parameter from the deesser plugin is now saved to the preset file.
- Improved band splitting for crystalizer with new default intensity values.
- Depending on the input gain or output gain values the corresponding level bars could not be aligned.
- When adding more equalizer bands they are set to `Bell` instead of `Off`.
- Equalizer APO presets loading is now working properly on locales different than C.
- Improved linking management between port filter nodes in PipeWire.
- The crystalizer plugin signal amplification was too high before. It should be within more reasonable levels now.

 
## 6.0.3
### 2021-07-16


### Features:
- Improved the resampler used in the plugins that require one(like the rnnoise plugin)
- Updated translations

### Bug fixes:

- Setting multiple autoloading presets should be fine now
- Transient windows are now properly set for some plugins dialogs
- The convolver impulse response menu was improved to workaround an issue where the impulse files was not loaded when only one was available in the menu, see https://github.com/wwmm/easyeffects/issues/1011
- Fixed a bug that could make the pitch plugin to not be properly initialized
- The saturation warning should not displace its neighbor widgets anymore
- Fixed the locale in a few widgets
- Fixed wrong alignment in a few widgets

 
## 6.0.2
### 2021-07-11


### Features:
- The Loudness plugin is being used again for the reasons described at https://github.com/wwmm/easyeffects/issues/820. This means that http://drobilla.net/plugins/mda/Loudness is an optional dependency again.

### Bug fixes:

- Fixed a segmentation fault that happened when optional dependencies were not installed

 
## 6.0.1
### 2021-07-09


### Features:
- Improved equalizer interface.
- Now we use a sidechain LSP compressor that allows the user to select and external source as the sidechain input.
- We now support the LSP compressor `Boosting` mode.
- When `split-channels` is enabled in the equalizer the imported APO preset will be applied only to the channel being visualized in the window. This will allow to import different presets for each channel instead of just settings the same values to both.

### Bug fixes:

- Fixed some segmentation faults that could happen when creating a preset autoloading profile or removing presets

 
## 6.0.0
### 2021-07-07


### Features:
- This is one of the biggest releases that I have ever made. The amount of changes is so big that it is hard to talk about everything here.
- The following are just the most import ones. People interested on the journey that got us here can take a look at https://github.com/wwmm/easyeffects/issues/904 and https://github.com/wwmm/easyeffects/issues/874.
- The application and its repository have been renamed from PulseEffects to `EasyEffects`
- gtkmm3 was replaced by gtkmm4
- Gstreamer was replaced by native PipeWire filters.
- Many features were reimplemented from scratch. The preset autoloading is one example. Another remarkable change will be seen in the plugins selection menu. Now the user can show in the window only the plugins that he/she wants to use.
- Boost is no longer a dependency. The price paid for that was a little change in our presets structures. With some patience it is possible to edit PulseEffects presets in a text editor and make them work in EasyEffects. Hopefully someone will come up with a script for this in the feature.
- New libraries are being used and some of the librarires that were optional before are now required
