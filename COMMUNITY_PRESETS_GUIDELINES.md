# Introduction to Easy Effects Community Presets

From `7.2.0` version, Easy Effects introduces the new system of Community
Presets which is an easy way for users to try and use presets made by the
community.

The Presets menu popover of the graphical application shows an additional tab
where community presets are listed.
The difference with Local Presets is that community ones are intended to be
easily installed since not all users were able to find presets on the internet,
which were practically reported in one of the subpage of the Github project
(that linked other repositories where the user should manually download them on
the local storage).
Now it should be easier to discover them making a search on the repository or
your favorite distribution.

Instead the Local Presets list is intended to save self-made presets and/or a
selected choice of presets imported by the community list.
Importing presets locally is essential when the user wants to autoload and
associate them to a specific device.
Indeed, Community Presets **cannot be autoloaded**.

In order to list the Community Presets, the user should install one or more
packages made and maintained by other Easy Effects users.
The packages can be shipped by Linux distributions in their specific
repositories.

# Guidelines for packagers of Easy Effects Community Presets

The followings are the guidelines that packagers should respect in order to make
their presets available inside Easy Effects.

## How to make a package

- The package should contain an `easyeffects` directory installed under one of
the paths listed in the `$XDG_DATA_DIRS` environment variable.
- The `easyeffects` directory above mentioned may contain the following
subfolders:
- - `output`: for presets files (with `.json` extension) applied to output
devices (showed only in the popover of the Output window).
- - `input`: for presets files (with `.json` extension) applied to input
devices (showed only in the popover of the Input window).
- - `irs`: for impulse response files (with `.irs` extension) used by the
Convolver effect.
- - `rnnoise`: for model files (with `.rnnn` extension) used by the
Noise Reduction effect.
- The folders above mentioned should contain a subdirectory with the name
associated to the installed package. In example:
- - `XDG_DATA_DIR/easyeffects/output/package-name`
- The name of the package should be something descriptive e.g.
`easyeffects-presets-LoudnessEqualizer`. If your package contains a group of
presets without a particular "theme", naming a package
`easyeffects-USERNAME-presets` is reasonable.

<!--
### Flatpak

Flatpak extension packages do not use `$XDG_DATA_DIRS`, and instead place and
search for files under a different directory (detailed in examples below).
Besides this detail, Flatpak and distribution packages should behave the same in
regard to community presets.

In order to publish a Flatpak preset package on Flathub you do the following
general steps:

1. Create AppStream MetaInfo file (should be stored in your preset repository as
it is not Flatpak-specific).
2. Create Flatpak json manifest file
3. Submit to Flathub, described following
[Flathub's guide](https://docs.flathub.org/docs/for-app-authors/submission/).

#### Step by step instructions

1. Decide on a name. It should be e.g.
`io.github.wwmm.easyeffects.Presets.PRESET_PACKAGE_NAME`. For example
`io.github.wwmm.easyeffects.Presets.LoudnessEqualizer` for a specific preset or
`io.github.wwmm.easyeffects.Presets.wwmm` for a group of presets without a
particular "theme". The name must be consistent across the files used for
Flatpak otherwise the package will not build/work properly.

> [!NOTE]  
> Flatpak uses `io.github.wwmm.easyeffects.Presets` as the extension point name
(which preset packages must use), while `com.github.wwmm.easyeffects` is the
name of the Easy Effects package itself. This is necessary since `com.github.*`
is only allowed for backwards compatibility reasons on Flathub, and newer
packages must use `io.github.*`.

2. Clone the Flathub repo for new submissions following
[Flathub's guide](https://docs.flathub.org/docs/for-app-authors/submission/).

3. Create the AppStream MetaInfo file, which should go in your preset
repository, not the Flathub repository.
Name it `io.github.wwmm.easyeffects.Presets.PRESET_PACKAGE_NAME.metainfo.xml`.
Replace `PRESET_PACKAGE_NAME`, `PRESET_PACKAGE_NAME_PRETTY`,
`DEVELOPER_NAME_ID`, `DEVELOPER_NAME`, and `REPO_URL`.
You may optionally add more information to this file (which may help improve
visibility of the package on Flathub/software stores) as described in the
[appstream docs](https://www.freedesktop.org/software/appstream/docs/).

```xml
<?xml version="1.0" encoding="UTF-8"?>
<component type="addon">
  <id>io.github.wwmm.easyeffects.Presets.PRESET_PACKAGE_NAME</id>
  <extends>com.github.wwmm.easyeffects</extends>
  <name>PRESET_PACKAGE_NAME_PRETTY Easy Effects Presets</name>
  <developer id="DEVELOPER_NAME_ID">
    <name>DEVELOPER_NAME</name>
  </developer>
  <summary>Some helpful and brief summary which gives crucial information</summary>
  <url type="homepage">REPO_URL</url>
  <url type="vcs-browser">REPO_URL</url>
  <url type="help">REPO_URL</url>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>MIT</project_license>
</component>
```

4. Create the Flatpak manifest file named
`io.github.wwmm.easyeffects.Presets.PRESET_PACKAGE_NAME.json`, this should go in
your Flathub repository. Replace `PRESET_PACKAGE_NAME`, `PRESET_FILE_NAME`,
`REPO_NAME`, `REPO_URL`, and `LATEST_COMMIT`.
An example of `REPO_NAME` is `wwmm/easyeffects` and `REPO_URL` could be
`https://github.com/wwmm/easyeffects.git`.
You can add more `install` commands in `build-commands` if you want to install
multiple presets. Make sure you carefully install to the correct directory for
each type of preset, with the options of `input`, `output`, `irs`, and
`rnnoise`. If the preset repo is not on GitHub remove or replace the
`x-checker-data` section.

```json
{
    "id": "io.github.wwmm.easyeffects.Presets.PRESET_PACKAGE_NAME",
    "runtime": "com.github.wwmm.easyeffects",
    "sdk": "org.freedesktop.Sdk//23.08",
    "branch": "stable",
    "runtime-version": "stable",
    "build-extension": true,
    "separate-locales": false,
    "modules": [
        {
            "name": "presets",
            "buildsystem": "simple",
            "build-commands": [
                "install -Dm644 io.github.wwmm.easyeffects.Presets.PRESET_PACKAGE_NAME.metainfo.xml -t ${FLATPAK_DEST}/share/metainfo",
                "install -Dm644 PRESET_FILE_NAME.json -t ${FLATPAK_DEST}/input/PRESET_PACKAGE_NAME",
                "install -Dm644 PRESET_FILE_NAME.json -t ${FLATPAK_DEST}/output/PRESET_PACKAGE_NAME",
                "install -Dm644 PRESET_FILE_NAME.json -t ${FLATPAK_DEST}/irs/PRESET_PACKAGE_NAME",
                "install -Dm644 PRESET_FILE_NAME.json -t ${FLATPAK_DEST}/rnnoise/PRESET_PACKAGE_NAME"
            ],
            "sources": [
                {
                    "type": "git",
                    "url": "REPO_URL",
                    "commit": "LATEST_COMMIT",
                    "x-checker-data": {
                        "type": "json",
                        "url": "https://api.github.com/repos/REPO_NAME/commits",
                        "commit-query": "first( .[].sha )",
                        "version-query": "first( .[].sha )",
                        "timestamp-query": "first( .[].commit.committer.date )"
                    }
                }
            ]
        }
    ]
}

```

5. It is also necessary to add the following file called `flathub.json` in the
Flathub repo. The skip icons check is stricly necessary, since unlike a normal
app we are not providing icons.
We also recommend enabling a Flathub bot to automatically merge PRs with updates
from the upstream repo. Given these are only preset files, this should not be a
very risky thing to do and avoids manual maintenance hassle.

```json
{
    "skip-icons-check": true,
    "automerge-flathubbot-prs": true
}

```

6. Now you can submit this to Flathub via a PR, following
[Flathub's instructions](https://docs.flathub.org/docs/for-app-authors/submission/).
-->

## Guidelines for package directories structure

The package name should be unique to avoid misunderstandings between different
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

### How Easy Effects looks for additional files
The package may contain also impulse responses and models for presets with
Convolver and Noise Reduction (RNNoise) effects.

They should be placed in the package structure following the same scheme
reported above. For example:
- `XDG_DATA_DIR/easyeffects/irs/wwmm-presets/impulse.irs`
- `XDG_DATA_DIR/easyeffects/rnnoise/wwmm-presets/model.rnnn`

From version `7.2.0` the mechanism to detect those files has been changed.
If you are going to ship a preset containing Convolver and/or RNNoise
with specific associated files, make sure to include those files inside the
package. But, more importantly, **make sure to build/save the preset with a
version >=`7.2.0`**.

More in detail, Easy Effects presets does not save the path anymore, but only
the name of the additional files (without extension).
Previously, the path was stored in the preset JSON file as `kernel-path`, and
now the replacement key which only stores the name is `kernel-name`.
So if you see `kernel-path` in a preset JSON file it probably needs to be
updated by saving the preset with a version >=`7.2.0`.

When a preset is loaded,
the application searches the specified file, first in the local directory,
then in `XDG_DATA_DIR/easyeffects`.
Therefore it's recommended to use a unique name for additional files. If you
ship `model.irs` inside your package and the user has a different impulse
response with the same filename locally, they will listen something different
when trying you preset.
