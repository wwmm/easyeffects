#!/usr/bin/env python3
"""Exercise CLI controls against a real instance on an isolated PipeWire server.

Run: python3 tests/cli_controls.py build/src/easyeffects
Requires pipewire, wireplumber, pw-cli, pw-dump and dbus-run-session.
"""

import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time


def wait_for(predicate, description):
    deadline = time.monotonic() + 20
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.1)
    raise AssertionError(f"Timed out waiting for {description}")


def run_test(binary, root):
    processes = []
    logs = []

    def start(*args):
        log = open(root / f"{len(processes)}.log", "w+")
        logs.append(log)
        process = subprocess.Popen(args, stdout=log, stderr=log)
        processes.append(process)
        return process

    def cli(*args):
        return subprocess.run([binary, *args], capture_output=True, text=True, timeout=10)

    def request(command, reply=False):
        with socket.socket(socket.AF_UNIX) as client:
            client.settimeout(5)
            client.connect(str(root / "runtime/EasyEffectsServer"))
            client.sendall((command + "\n").encode())
            return client.recv(1024).decode() if reply else None

    def graph():
        return json.loads(subprocess.check_output(["pw-dump"], timeout=10))

    def has_link(source, target):
        objects = graph()
        names = {o["id"]: o.get("info", {}).get("props", {}).get("node.name")
                 for o in objects if o["type"].endswith(":Node")}
        return any(names.get(o["info"]["output-node-id"]) == source
                   and names.get(o["info"]["input-node-id"]) == target
                   for o in objects if o["type"].endswith(":Link"))

    try:
        start("pipewire")
        wait_for(lambda: (root / "runtime/pipewire-0").exists(), "PipeWire")
        start("wireplumber", "--profile", "policy")
        for name, media_class in (("test_sink", "Audio/Sink"), ("test_source", "Audio/Source/Virtual")):
            subprocess.run(["pw-cli", "create-node", "adapter",
                            f'{{ factory.name=support.null-audio-sink node.name={name} '
                            f'media.class={media_class} object.linger=true audio.position=[FL FR] }}'],
                           check=True, capture_output=True, timeout=10)
        wait_for(lambda: len([o for o in graph() if o["type"].endswith(":Port")]) >= 8,
                 "test device ports")
        config = root / "config/easyeffects/db/easyeffectsrc"
        config.parent.mkdir(parents=True)
        # Initial fixture only: all subsequent changes go through the live server.
        config.write_text("[EffectsPipelines]\ninactivityTimerEnable=false\nprocessAllInputs=false\n"
                          "[StreamInputs]\ninputDevice=test_source\nuseDefaultInputDevice=false\n"
                          "[StreamOutputs]\noutputDevice=test_sink\nuseDefaultOutputDevice=false\n")
        server = start(binary, "--service-mode", "--hide-window")
        wait_for(lambda: (root / "runtime/EasyEffectsServer").exists(), "EasyEffects")
        wait_for(lambda: has_link("ee_soe_output_level", "test_sink"), "output pipeline")
        help_text = cli("--help").stdout
        for option, tag, source, target in (
            ("microphone-monitoring", "microphone_monitoring", "easyeffects_source", "test_sink"),
            ("audio-sharing", "audio_sharing", "ee_soe_output_level", "easyeffects_source"),
            ("bypass", "global_bypass", None, None),
        ):
            assert f"--{option}" in help_text
            assert f"--{option}-toggle" in help_text
            for value in ("2", "1", "1", "2"):
                assert cli(f"--{option}", value).returncode == 0
                wait_for(lambda: request(f"get_{tag}", True) == value, f"{option} state {value}")
                assert cli(f"--{option}", "3").stdout == value + "\n"
                if source:
                    wait_for(lambda: has_link(source, target) == (value == "1"), f"{option} routing")
            for expected in ("1", "2"):
                assert cli(f"--{option}-toggle").returncode == 0
                wait_for(lambda: cli(f"--{option}", "3").stdout == expected + "\n", f"{option} toggle")
            # Change the live state through another client, then query via CLI.
            request(f"{tag}:1")
            wait_for(lambda: cli(f"--{option}", "3").stdout == "1\n", "live query")
            if option != "bypass":
                for bad in ("2", "3", "true", "1junk"):
                    request(f"{tag}:{bad}")
                    assert request(f"get_{tag}", True) == "1"
                invalid = cli(f"--{option}", "invalid")
                assert "invalid" in invalid.stdout
                assert cli(f"--{option}", "3").stdout == "1\n"
                assert cli(f"--{option}").returncode != 0
            cli(f"--{option}", "2")
            assert server.poll() is None
            print(f"PASS: {option} set/query/toggle and live state", flush=True)
        cli("--quit")
        server.wait(timeout=10)
        print("PASS: isolated CLI integration", flush=True)
    except BaseException:
        for log in logs:
            log.flush()
            log.seek(0)
            print(log.read()[-5000:], file=sys.stderr)
        raise
    finally:
        for process in reversed(processes):
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
        for log in logs:
            log.close()


if __name__ == "__main__":
    binary = str(Path(sys.argv[1]).resolve())
    if len(sys.argv) == 3:
        run_test(binary, Path(sys.argv[2]))
    else:
        with tempfile.TemporaryDirectory(prefix="ee-cli-test-") as temporary:
            root = Path(temporary)
            for name in ("runtime", "config", "data", "cache", "state"):
                (root / name).mkdir(mode=0o700)
            env = os.environ.copy()
            for name in ("PULSE_SINK", "PULSE_SOURCE", "PIPEWIRE_REMOTE", "PIPEWIRE_RUNTIME_DIR"):
                env.pop(name, None)
            env.update({"XDG_RUNTIME_DIR": str(root / "runtime"),
                        "XDG_CONFIG_HOME": str(root / "config"),
                        "XDG_DATA_HOME": str(root / "data"),
                        "XDG_CACHE_HOME": str(root / "cache"),
                        "XDG_STATE_HOME": str(root / "state"),
                        "QT_QPA_PLATFORM": "offscreen", "LC_ALL": "C"})
            bus_config = root / "dbus.conf"
            bus_config.write_text('<busconfig><type>session</type><listen>unix:tmpdir=/tmp</listen>'
                                  '<auth>EXTERNAL</auth><policy context="default">'
                                  '<allow send_destination="*"/><allow receive_sender="*"/>'
                                  '<allow own="*"/></policy></busconfig>')
            result = subprocess.run(["dbus-run-session", "--config-file", str(bus_config), "--", sys.executable,
                                     str(Path(__file__).resolve()), binary, str(root)], env=env)
            sys.exit(result.returncode)
