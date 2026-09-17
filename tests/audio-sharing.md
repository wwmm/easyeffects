# Audio-sharing routing regression

```sh
python3 tests/audio_sharing.py build/src/easyeffects
```

Requires Python 3, PipeWire (`pipewire`, `pw-cli`, `pw-dump`, `pw-cat`, `pw-link`), WirePlumber's
`policy` profile, and `dbus-run-session`. The test runs a private audio server
with virtual devices and temporary settings; it does not use hardware devices.

The test checks microphone monitoring with sharing off and on, both monitoring
destinations, a reduced monitoring volume, inactivity, and shutdown cleanup.
Separate 1 kHz microphone and 2 kHz desktop tones verify that sharing preserves
local desktop volume and that monitoring volume does not change recorded mic volume.
The virtual source must never feed the monitor: that would replay shared desktop
audio at the speakers, or feed it back into output effects.
