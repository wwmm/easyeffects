# Introduction to Easy Effects Community Presets

From `7.2.0` version, Easy Effects introduces the new system of Community
Presets which is an easy way for users to try and use presets made by the
community.

The Presets menu popover of the graphical application shows an additional tab
where community presets are listed.
The difference with Local Presets is that  community ones are intended to be
easily installed since not all users were able  to find presets on the internet,
which were practically reported in one of the  subpage of the Github project
(that linked other repositories where the user  should manually download them on
the local storage).
Now it should be easier to discover them making a search on Flatpak or the
repository or your favorite distribution.

Instead the Local Presets list is intended to save self-made presets and/or a
selected choice of presets imported by the community list.
Importing presets locally is essential when the user wants to autoload and
associate them to a  specific device.
Indeed, Community Presets **cannot be autoloaded**.

In order to list the Community Presets, the user should install one or more
packages made and maintained by other Easy Effects users.
The packages can be shipped by Linux distributions in their specific
repositories or by extensions available on Flatpak.

# Guidelines for packagers of Easy Effects Community Presets

The followings are the guidelines that packagers should respect in order to make
their presets available inside Easy Effects.

## How to make a package

- The package should contain an `easyeffects` directory installed under one of
the paths listed in the `$XDG_DATA_DIRS` environment variable.
- The `easyeffects` directory above mentioned should contain the following
subfolders:
- - `output`: for presets applied to output devices (showed only in the popover
of the Output page).
- - `input`: for presets applied to input devices (showed only in the popover of
the Input page).
- The input/output folders above mentioned should contain a subdirectory with
the name associated to the installed package. In example:
- - `XDG_DATA_DIR/easyeffects/output/package-name`

## Guidelines for package directories structure

The package name should be unique to avoid misunderstanding between different
packages. Therefore try to use a name which is exclusive to your package.
Easy Effects can still distinguish and list presets from packages having the
same name but installed in different `XDG_DATA_DIR`.
Nevertheless this would confuse users seeing multiple presets with the same
package name in the menu list. For this reasons we recommend to try to use
unique package names.
So choose a name that you know it won't be used by other packagers, maybe
including your nickname.

In example:
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets`
- `XDG_DATA_DIR/easyeffects/output/Digitalone`
- `XDG_DATA_DIR/easyeffects/output/vchernin`

### How Easy Effects looks for Community Presets
Basically Easy Effects scans every directory with the package name looking for:
- Files with the `.json` extention.
- - The packager should take care of the integrity of those `.json` files being
text files that can be correctly parsed by the application in order to be loaded
without errors.  
- The scan takes place only in the first and the second level of the directory
with the package name. In other words, the presets files contained in the folder
itself and its subfolder.

In example, the following presets are correctly loaded and listed:
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/generic-pop.json`
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/generic-rock.json`
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/device-1/pop.json`
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/device-1/rock.json`
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/device-2/pop.json`
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/device-1/rock.json`

Files with the wrong extention or presets placed more in depth are not shown.
In example, the followings are ignored:
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/rock.txt`
- `XDG_DATA_DIR/easyeffects/output/wwmm-presets/device-3/alternative/rock.json`
