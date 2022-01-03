# Changelog
## 6.1.5
              
### 2021-11-17

### Features:
- It is now possible to combine impulse responses in the Convolver interface. A new impulse file is generated and it should be visible in the impulse list.
- Improved `x axis` drawings in our plots. Now the number of labels is adjusted dynamically depending on our window width.
- The documentation has been updated reflecting the new EasyEffects features. Old references about PulseEffects have been removed. The documentation button has been added in the menu section.

### Bugfixes:
- When a spinbutton is filled with an out of range value, now it is updated with the lowest/highest value rather than resetting to the previous one.
- The application window now remembers the maximized state and restores it on the next opening event.

### Notes:
- The `tbb` library is a new dependency https://www.threadingbuildingblocks.org

## 6.1.4
              
### 2021-10-16

### Features:
- The Limiter and the Multiband Compressor plugins can now use an optional external sidechain.
- The Autogain plugin now allows the user to select which Loudness is used as reference for the volume correction.
- The APO Profile Import feature of Equalizer plugin now parses the "Pre Amplification" parameter.
- Optional Cubic Volume can be enabled in General Settings.

### Bugfixes:
- The Spectrum plugin was supposed to enter passthrough whenever it was not visible, but this was happening only when our window was closed.
- Improved support for Assistive Technology.
- The probes used in some filters like the Compressor and the Limiter were not being relinked after changing the order of the plugins in the pipeline. It should be working now.
  
## 6.1.3
              
### 2021-10-03

### Features:
- PipeWire monitor streams are now excluded and removed from the applications list.

### Bugfixes:
- Hopefully crashes like the one reported at [1172](https://github.com/wwmm/easyeffects/issues/1172) are fixed.
- Prevented a case in which Spectrum was crashing.
- Pavucontrol is not added anymore to input applications list on systems with localization different than English.

## 6.1.2
              
### 2021-09-20

### Features:
- Improved compatibility with WirePlumber. This is needed to run on systems that decided to use it instead of the
  built-in PipeWire session manager. More information at issue [1144](https://github.com/wwmm/easyeffects/issues/1144).

## 6.1.1
              
### 2021-09-17

### Features:
- When trying to add an autoloading profile for a device already in the list its target preset will be updated. This way we can change the profile preset without having to remove and recreating it.
- The preset autoloading support implementation was redesigned again. It should work on more hardware now. For more information see issue [1051](https://github.com/wwmm/easyeffects/issues/1051).
- If the Limiter or the Maximizer are set in the last position of the plugin stack, new plugins are added at the second to last position in order to prevent hardware damage on eventually high output level.
- Removing an application from the blocklist, its previous enabled state is restored.

### Bugfixes:
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

### Bugfixes:
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

### Bugfixes:
- Setting multiple autoloading presets should be fine now
- Transient windows are now properly set for some plugins dialogs
- The convolver impulse response menu was improved to workaround an issue where the impulse files was not loaded when only one was available in the menu https://github.com/wwmm/easyeffects/issues/1011
- Fixed a bug that could make the pitch plugin to not be properly initialized
- The saturation warning should not displace its neighbor widgets anymore
- Fixed the locale in a few widgets
- Fixed wrong alignment in a few widgets

## 6.0.2
              
### 2021-07-11

### Features:
- The Loudness plugin is being used again for the reasons described at https://github.com/wwmm/easyeffects/issues/820. This means that http://drobilla.net/plugins/mda/Loudness is an optional dependency again.

### Bugfixes:
- Fixed a segmentation fault that happened when optional dependencies were not installed

## 6.0.1
              
### 2021-07-09

### Features:
- Improved equalizer interface.
- Now we use a sidechain LSP compressor that allows the user to select and external source as the sidechain input.
- We now support the LSP compressor `Boosting` mode.
- When `split-channels` is enabled in the equalizer the imported APO preset will be applied only to the channel being visualized in the window. This will allow to import different presets for each channel instead of just settings the same values to both.

### Bugfixes:
- Fixed some segmentation faults that could happen when creating a preset autoloading profile or removing presets

## 6.0.0
              
### 2021-07-07

### Features:
- This is one of the biggest releases that I have ever made. The amount of changes is so big that it is hard to talk about everything here. 
- The following are just the most import ones. People interested on the journey that got us here can take
- a look at https://github.com/wwmm/easyeffects/issues/904 and https://github.com/wwmm/easyeffects/issues/874.

### Features:
- The application and its repository have been renamed from PulseEffects to `EasyEffects`
- gtkmm3 was replaced by gtkmm4
- Gstreamer was replaced by native PipeWire filters.
- Many features were reimplemented from scratch. The preset autoloading is one example. Another remarkable change will be seen in the plugins selection menu. Now the user can show in the window only the plugins that he/she wants to use.

### Notes:
- Boost is no longer a dependency. The price paid for that was a little change in our presets structures. With some patience it is possible to edit PulseEffects presets in a text editor and make them work in EasyEffects. Hopefully someone will come up with a script for this in the feature.
- New libraries are being used and some of the librarires that were optional before are now required
