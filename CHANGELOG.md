# Changelog

## [Unreleased]
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
