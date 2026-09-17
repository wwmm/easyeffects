# CLI integration test

Build Easy Effects, then run:

```sh
python3 tests/cli_controls.py build/src/easyeffects
```

The test requires Python 3, PipeWire (`pipewire`, `pw-cli`, `pw-dump`),
WirePlumber (with the `policy` profile), and `dbus-run-session`.
It starts a private D-Bus session, PipeWire server, virtual audio devices, and
Easy Effects instance with temporary XDG directories. The session manager runs
without hardware monitors, so the test does not use the user's sound devices.

A recording socket peer also verifies that the new controls send no window
visibility commands unless `--hide-window` is explicitly requested.

It checks help, enable/disable/query/toggle commands, repeated setters, malformed
requests, changes from another socket client, and the resulting live PipeWire
links. Bypass is exercised as a compatibility baseline. Configuration fixtures
are written only before startup; all state changes use the running instance.
