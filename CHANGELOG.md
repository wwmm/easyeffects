# Changelog

## [Unreleased]

## [4.6.8]

### Added

- It is now possible to set different buffer and latency values for the pulsesrc and the pulsesink plugins. This
  means that there is more room to reduce the overall latency. Pulsesrc is at the start of the effects pipeline and
  pulsesink at its end.
- Updated Slovak, Italian and Turkish translations.

### Fixed

- The auto gain `detect-silence` key was not being correctly saved to the preset file.

## [4.6.7]

### Added

- The autogain plugin now has a button to reset its history and another that allows silence detection to be disabled.
  Disabling this detection helps with issues like the ones in https://github.com/wwmm/pulseeffects/issues/550.

### Fixed

- Fixed a bug that prevented loopback devices from being shown in PulseEffects Applications section. Now it is possible
  to apply effects for audio sent from mobile devices through a bluetooth connection. It should work for other kinds of
  loopbacks too.

## [4.6.6]

### Added

- Improved preset auto loading. This should fix https://github.com/wwmm/pulseeffects/issues/520
- Improved built-in applications blacklist. We now use the applications id where possible as they are not changed by
  translations. This should avoid Pavucontrol probes being shown in PE window.

## [4.6.5]

### Added

- Delaying preset auto loading by 3 seconds. This should help to fix the issue reported
  at https://github.com/wwmm/pulseeffects/issues/520

### Fixed

- When PE was executed in an environment without Pulseaudio executable in the system
  path(like flatpak) a crash would happen because of unhandled execeptions. This was
  fixed PE should not crash anymore when Pulseaudio exe is not found.

## [4.6.4]

### Removed

- The reset settings popup was removed. It does not work reliably outside of Gnome.
  On other desktops the popup may be shown whenever PE is started. Even if the user
  select yes or no. As resetting settings after an upgrade hasn't been necessary for
  a while I removed this feature.

## [4.6.3]

### Fixed

- Depending on the hardware we could crash when trying to show the channel
  mapping in use by Pulseaudio.
  See https://github.com/wwmm/pulseeffects/pull/522. This was fixed.

### Changed

- Maximizer plugin now uses 25 ms as default release value.

## [4.6.2]

### Added

- Improved Pipeline state switching when audio applications alternate between
  the playing and paused state or skip tracks. These operations should cause less
  problems like the ones saw here https://github.com/wwmm/pulseeffects/issues/504.

### Fixed

- Loading presets using the command line does not result in a crash anymore.
- Fixed a bug that made PulseEffects crash in systems with custom Pulseaudio
  version string.
- Changing the blocksize while our pipeline is playing should not freeze
  PulseEffects anymore.
- The command line option used to list available presets was not working
  properly. This was fixed.

## [4.6.1]

### Added

- Pulseaudio avoid-resampling feature is now supported. You will need Pulseaudio
  13 or later for that.
- We now show in the interface Pulseaudio information that you would usually
  need to use command line tools or opening its configuration files to see. Things
  like which modules are loaded and the current configuration that its daemon is
  using.

### Fixed

- Changing convolver files should not make us crash anymore.
- Changing applications volume will not result in noises anymore. You will need
  Pulseaudio 13 or later for that.

## [4.6.0]

### Added

- Updated Indonesian and Italian translations.
- Presets can now be automatically loaded when wired headphones are plugged in
  the sound card jack. This did not work before because wired headphones do not
  create a new output device. They are shown as a port in the current default
  output device. This feature comes with a price. People may have to redefine
  their automatic preset loading settings.
- Ctrl+Q closes the window.

## [4.5.9]

### Added

- Drag and drop can be used to change effects order. Take a look at
  https://github.com/wwmm/pulseeffects/issues/447 for more information.
- Improved maximizer and compressor help
- Improved Italian translations

### Fixed

- fixed wrong release unit in maximizer interface

### Removed

- Removed the destructive-action class from the "remove buttons". This used to
  make them red but in recent GTK versions this has been a little buggy. Sometimes
  it works sometimes it doesn't...

## [4.5.8]

### Fixed

- Fixed problems in pt_BR translation that could make installation fail in
  some systems.

## [4.5.7]

### Added

- Updated Czech and Turkish translations.
- Linux Studio Plugins `1.1.7` has new filters in its parametric equalizer that
  are implemented just like the biquad filters found in Equalizer APO. A very
  popular equalizer used in Windows. This will help PulseEffects users to
  properly use headphone correction curves that were designed with Equalizer APO
  filters in mind. Just choose `APO (DR)` as mode in each band menu. You can have
  more information about why this is a useful feature here
  https://github.com/wwmm/pulseeffects/issues/421 and here
  https://github.com/sadko4u/lsp-plugins/issues/29.

### Note for packagers

- PulseEffects will probably still work with older Linux Studio Plugins versions.
  But I would expect some errors printed in the system log if the user tries to
  choose the APO filter.

## [4.5.6]

### Added

- Chinese and Turkish translations.
- Improvements to the Crystalizer plugin. It now shows the loudness range before
  and after processing. Some improvements to its algorithm were also done.
  Latency and memory usage were reduced and it now has a toggle button to turn
  on/off an "Aggressive Mode".

## [4.5.5]

### Fixed

- Fixed a memory leak that could happen in the crystalizer plugin in some cases.

## [4.5.4]

### Added

- Improved crystalizer algorithm. Some noises that could be created in a few
  cases do not happen anymore.
- Improved Italian translations.

## [4.5.3]

### Added

- Improved crystalizer algorithm. Its CPU usage was reduced.
- Improved Italian translations.

## [4.5.2]

### Added

- Improved crystalizer algorithm. It should preserve more of the original wave
  shape when increasing the dynamic range.

## [4.5.1]

### Added

- Updated help and translations.
- Reworked crystalizer plugin. It now has more band filters.
- Block size is again a global setting because besides the convolver
  crystalizer now also needs audio buffer sizes following a power of 2. As before
  higher block sizes will increase latency but will reduce cpu usage in plugins
  like the autogain.
- The spectrum now has line mode and a switch to enable/disable linear
  gradient.

### Changed

- The compressor(not the multiband) is now from the Linux Studio Plugins package.

## [4.5.0]

- Warning: This is not one more of the typical harmless updates I usually do.
  A few of the many features introduced required a few changes to the preset
  format. It is not a total break like the last time(still using json) but manual
  intervention will be necessary in order to use values from the old format. With
  the exception of the equalizer it is just a matter of doing some copy and paste
  in a text editor.

### Added

- It is now possible to make a preset autoload when an input or output device is
  plugged in the system.
- The Crystalizer plugin now splits the frequency spectrum in three regions. This
  allows the application of a smaller intensity to high frequencies.
- Remember window size.
- Updated help and translations.
- Added options to show/hide bars borders in the spectrum and to change their
  line widths.
- Improved dynamic pipeline.

### Changed

- We now use Linux Studio Plugins "Parametric Equalizer x32 LeftRight" (LV2 version)
  http://lsp-plug.in/?page=manuals&section=para_equalizer_x32_lr instead
  of the one from Gstreamer. Reason: it has more features and supports different
  settings for each channel natively. Besides the usual high-pass and low-pass
  it has a notch and a resonant filter. It is also possible to mute
  each band on the fly and change its filter slope. There is also a "solo" mode
  where we can easily listen to the effect caused by a specific band without
  having to manually disable all the others.
- The `Enable All Apps` function was split in two. We now have
  `Process All Outputs` and `Process All Inputs`. This way the user can choose
  to automatically apply effects to the output of all apps while manually choosing
  which ones will have their input processed.

### Fixed

- Icon is properly rendered in QT environments
- Custom spectrum height is applied on startup

## [4.4.7]

### Added

- Updated Indonesian and Czech localizations.
- Added the delay compensator plugin from Linux Studio Plugins (LV2 library).
  It is an optional plugin. We used it in the past but it was removed because its
  license was not as friendly as I would like. Now that their license has changed
  we are using it again.

## [4.4.6]

### Fixed

- When the user loaded a preset file pointing to a impulse file that did not
  exist the convolver would consume memory crazily while trying to do its setup
  multiple times. This should not happen anymore.

## [4.4.5]

### Fixed

- In some situations where audio devices were added and removed repeatedly very
  fast a crash would happen when updating the device list in the interface. Read
  this issue for more information https://github.com/wwmm/pulseeffects/issues/408
  This should not cause a crash anymore.

## [4.4.4]

Last release of 2018! Happy new year! :D

### Added

- The pitch plugin can be used in the applications output too besides the
  microphone.
- Updated Italian translations.

## [4.4.3]

### Added

- Added a few preprocessor directives to disable at compile time api that only
  exists on Linux. Now it should compile on Freebsd.

## [4.4.2]

### Added

- It is now possible to change the realtime priority or the niceness value. We do
  this using RealtimeKit just like Pulseaudio. By default we do not change any of
  these values. It is up to the user to enable this in the settings menu. Only the
  Gstreamer threads responsible for the audio processing have their priorities
  changed. Users wanting to verify if the changes took effect can do this using
  `ps -m -l pid` where pid is PulseEffects process pid.
- Now the user is able to apply different equalizations for the left and the right
  channels. Just enable the option `Split Channels` in the equalizer menu. This
  feature is particularly interesting for people with hearing loss. Take a look at
  this issue opened in our page https://github.com/wwmm/pulseeffects/issues/353 to
  know more.
- The spectrum has more customization options.
- The applications blacklist is saved to the user preset.
- When alternating presets a large disk activity was generated. In order to
  avoid this we now check if each gsettings key really needs to be updated. As
  gsettings does not do that we had to implement our own checks.
- Our interface should be a littler closer to the Gnome HIG guidelines.

### Changed

- Effects are by default switched on to all audio applications. This can be
  disabled in the settings menu.

### Fixed

- When installed under /usr/local PulseEffects would crash because of not found
  plugins. This was fixed.

## [4.4.1]

### Added

- Updated translations
- Improved dynamic pipeline. Alternating presets while playing should cause
  less crashes.
- Improved the adapter plugin used internally by PulseEffects
- Using std::async instead of std::thread to do a few background tasks like
  loading an impulse response file

### Fixed

- Fixed presets menu button label not being translated after a reset. It was
  always being changed to the english label "Presets"

## [4.4.0]

### Added

- Better error handling and logs. This will help to remove to remove the
  segmentation faults a few people were having and with lucky it will be easier
  to find the source of others.

## [4.3.9]

### Fixed

- Fixed a bug that could cause our pipeline to crash when changing the effects
  order.

## [4.3.8]

### Fixed

- Fixed a few sources of segmentation faults I could verify and hopefully a few
  others that do not happen in my computer.

## [4.3.7]

### Fixed

- Using GST_PAD_PROBE_TYPE_IDLE instead of GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM.
  This seems to fix a rare and hard to find bug where our pipeline crashed with
  a no linked error when the user switched presets while PulseEffects was paused.

## [4.3.6]

### Added

- When no application is playing audio our GStreamer pipeline is now put in the
  paused state instead of the null state. This should remove undesired choppy
  sounds that could be listened whenever we restarted the audio processing.

### Fixed

- Sometimes the wrong sampling rate could be shown in the headerbar info. It was
  fixed.

## [4.3.5]

### Added

- More space at the right of the up/down buttons used to change effects order.
  In a few systems GTK is configured to always show the scrollbar. In this case
  the down button would be partially hidden.
- PulseEffects can be reset from the command line "pulseeffects -r".

### Removed

- The app menu with the entries quit, about and help was removed. This menu was
  deprecated by GTK developers so we are just following their guideline. The about
  button can be found in the general section of the main menu (hamburger).

## [4.3.4]

### Added

- PulseEffects is now dbus activatable. Besides improving user experience with
  gnotifications this also brings the benefit of making the first contact of the
  user with PulseEffects better. Usually they expect the effects to keep running
  when the window is closed. But up to now you had to enable
  "Start Service at Login" and do a logout for this to happen. Now that we are
  dbus activatable PE will be launched as service and show its window when the
  user clicks in the PE icon. Effects will remain when the window is closed. But
  note that enabling "Start Service at Login" is still necessary for autostarting
  PE in the next login.
- A verification was added to verify if the current plugin list has any invalid
  plugin name. If there is the list is reset to its default value.

## [4.3.3]

### Added

- A system notification (gnotification) is show to the user asking if he/she
  wants to reset PulseEffects settings after an update.

## [4.3.2]

### Added

- Settings are automatically reset after an upgrade. If you want to know the
  reason why take a look at the issues #327, #318 and #293 in PulseEffects github
  page. A few users have configuration problems after an upgrade that are only
  solved after resetting PulseEffects settings. In all of the cases I could not
  reproduce the issue they had in my computers. As in some of the cases the crash
  was so severe that the only way to reset was using dconf in command line I
  decided to implement an automatic reset after upgrades or downgrades. As almost
  all PulseEffects settings are saved to the presets file this shouldn't be too
  annoying for the users.

## [4.3.1]

### Fixed

- One of the help images was not being installed

## [4.3.0]

### Added

- The about dialog window "hide" method is explicitly called now. This may fix
  an issue where this window was not closed for users not using gnome
- The user can blacklist applications he/she does not want effects being applied.
- We check if there is any forbidden character in the name of the preset that
  the user is adding. If there is we just clear the field and do not try to add
  the preset.

## [4.2.9]

### Added

- PulseEffects now remembers the custom input and output device selected by the
  user. In case it does not exists in the next start we fallback to the default
  device.

## [4.2.8]

### Added

- The latency introduced by the Gstreamer pipeline is displayed in the headerbar.

## [4.2.7]

### Added

- The adapter plugin used internally by PE reports the latency that it may
  introduce(proportional to the block size) to GStreamer. Latency is only added
  when the block size s larger than the number o samples per buffer in Pulseaudio
  buffer.

### Fixed

- Fixed a regression that could cause severe noises when multiple audio apps
  were playing at the same time.

## [4.2.6]

### Added

- Improved the plugin that adapts the size of the buffer given by pulsesrc.
- Improved log messages. The name of the audio app related to the failures will
  be printed together with the error messages.

## [4.2.5]

### Added

- Added a plugin that adapts the audio buffer size to a power of 2 value. We
  were using the block size property of the pulsesrc plugin before but it was not
  working well. Sometimes Pulseaudio was not able to give a buffer with the size
  we needed and some noises were created because of GStreamer buffer under flows.
  Now it should be possible to use higher block sizes without noises. A larger
  block may be interesting to users using weak processors as a few plugins use
  less CPU with larger block sizes. As always there is a trade-off. Large blocks
  increase latency.

## [4.2.4]

### Added

- New application icon created by ZenCubic(https://github.com/zencubic).

## [4.2.3]

### Added

- Improved the routine responsible to enable or disable audio processing
  depending on whether there is an audio application playing or not. The previous
  code wasted cpu resources and on rare occasions could freeze the interface.
- The sample format and sampling rate info shown in the headerbar had a few
  unicode characters replaced by icons. Some fonts do not have the characters that
  were being used.

## [4.2.2]

### Fixed

- Fixed a bug where the interface allowed the user to choose zero weights in
  the auto gain. The zero value was not passed to the plugin because of a few
  protections. So this bug did not affected plugin operation. But it makes
  gsettings crash whenever the value is set to zero and this leads to core dump
  when PE is closed.

## [4.2.1]

### Added

- Added an auto gain plugin based on libebur128. It replaces the previous
  auto volume feature. It has the advantage of targeting the perceived loudness
  level.

### Fixed

- Sometimes PulseEffects was not paused when there was no application playing
  audio. Fixed that.

### Note for Packagers

- libebur128-dev is now a build time dependency.

## [4.2.0]

### Added

- We finally have plugins documentation and a few basic explanations about some
  sections of the user interface. They can be accessed by clicking in the new help
  button next to the settings menu button or by clicking the F1 key.
- The interface shows a warning when the plugin input is saturated
  (you really do not want this to happen). The check mark next to the plugin label
  is temporarily switched to an exclamation mark while the input is saturated.

### Removed

- Removed the stereo enhancer. It is a little buggy and most if not all of what
  it does can be accomplished with the stereo tools.

### Note for Packagers

- itstool is now a build time dependency.

## [4.1.9]

### Added

- A crystalizer plugin. Useful to add a little of dynamic range to overly
  compressed songs. It is a port I made of the algorithm in FFMPEG crystalizer
  plugin to the GStreamer framework. So all the credits should go to the FFMPEG
  developers.
- Audio format and sampling rate of the main processing stages are shown in the
  GTK headerbar subtitle(below the title). In the section that applies effects for
  applications output the first info is about the null sink PulseEffects(apps).
  this is the place applications output are redirected to. The second information
  shows the format and sampling rate used by the plugins and PulseEffects pipeline.
  The third information shown is from the current default output device. In the
  section that applies effects for microphones the idea is the same but the first
  information show is the one from the source and the last one is about
  PulseEffects(mic) null sink. Applications like skype will record processed audio
  from this null sink monitor.

### Note for Packagers

- There is no need to add ffmpeg as dependency.

## [4.1.8]

### Added

- Improved convolver impulse response autogain calculation. Changing impulse
  responses on the fly should result in less abrupt volume changes now.
- A warning is shown in the convolver interface when the user loads a preset
  that points to an impulse file that does not exists in PulseEffects folder
  anymore.
- No limit on the number of frames used from impulse response file. It is up to
  the user to decided if his/her computer is strong enough to handle the impulse
  file loaded.

## [4.1.7]

### Added

- Improved Russian translations
- Using a background thread to load the impulse response file in the plugin.
  This avoids playback stalls when the impulse file is large.

### Fixed

- Fixed the meson scripts so that the convolver is optional. This would allow
  compilation in system that do not have zita-convolver.
- Removed a potential crash when the impulse response file was too small.
- Removed memory leaks in the convolver interface

## [4.1.6]

### Fixed

- PulseEffects now uses the system zita-convolver library. This was request by
  package managers and after a few exchange of ideas that can be seen here
  https://github.com/wwmm/pulseeffects/issues/262 I was able to make this move.
  Zita-convolver version 3 and 4 should work. With this fftwl is not a Pulseeffects
  dependency anymore but as the zita package needs it users will have to have it
  anyway.

## [4.1.5]

### Fixed

- Fixed a bug that would crash PulseEffects whenever the user tried to load an
  impulse response file with more than 2 channels(only stereo is supported).

## [4.1.4]

### Added

- A convolver plugin. It is the first plugin I have written :D :-) As audio
  processing is not my original field of expertise I prefer to use plugins from
  mature projects like Calf. But GStreamer support for LV2 plugins is limited and
  none of the available convolvers work with it. You can see more details here
  https://github.com/wwmm/pulseeffects/issues/164. As this situation will not
  change any time soon I had to put my hands in the dirty. The plugin is based on
  zita-convolver library and while writing it I took inspiration in the work of
  other people you can find here https://github.com/tomszilagyi/ir.lv2/ and here
  https://github.com/x42/convoLV2. For now only stereo impulse response files
  encoded as wav are supported.
- Import presets dialog now has filters to show only presets files (.json).
- The impulse response file import dialog also has a filter (.irs). In case your
  response file is "_.wav" just rename it to "_.irs".
- User can change Pulseaudio block size. In order to implement the convolver I
  had to force Pulseaudio block size to be a power of 2. As I am not sure if the
  move from an automatic size to a fixed size is going to bring problems to the
  users I decided to expose this setting so that the user can fine tune it in case
  there is any noise or other problem related to this parameter. It is also
  possible to set different block size values for the input effects pipeline and
  for the output effects pipeline. For some reason I don't know GStreamer's
  webrtcdsp plugin does not work with block sizes smaller than 512. So users
  applying effects for microphones while using webrtc should use 512 or above for
  this parameter.
- The convolver spectrum plot color follows the global spectrum color chosen by
  the user.

### Note for packagers

- There is no need to add zita-convolver library as dependency. Its source
  is shipped with PulseEffects and compiled by PulseEffects scripts. But zita
  dependencies should still be installed in the system. This means that the
  library fftw3f that is a Zita dependency is now also a PulseEffects dependency.
- gstreamer-fft-1.0 is now a dependency (impulse response file spectrum
  calculation).

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
  to the respective plugin control section. Let's hope this makes things easier
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
