#!/usr/bin/env python3
"""Check that audio sharing never feeds desktop audio back through mic monitoring.

Run: python3 tests/audio_sharing.py build/src/easyeffects
Requires pipewire, wireplumber, pw-cli, pw-dump, pw-cat, pw-link and dbus-run-session.
"""

import array
import math
import signal
import struct
import wave
import json
import os
from pathlib import Path
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

    def graph():
        return json.loads(subprocess.check_output(["pw-dump"], timeout=10))

    def has_link(source, target):
        objects = graph()
        names = {o["id"]: o.get("info", {}).get("props", {}).get("node.name")
                 for o in objects if o["type"].endswith(":Node")}
        return any(names.get(o["info"]["output-node-id"]) == source
                   and names.get(o["info"]["input-node-id"]) == target
                   for o in objects if o["type"].endswith(":Link"))

    def tone(path, frequency):
        with wave.open(str(path), "wb") as audio:
            audio.setparams((2, 2, 48000, 0, "NONE", ""))
            period = b"".join(struct.pack("<hh", *([int(3276 * math.sin(2 * math.pi * frequency * i / 48000))] * 2))
                              for i in range(48000))
            audio.writeframes(period * 30)

    def capture_amplitudes(device):
        path = root / "capture.wav"
        recorder = start("pw-cat", "--record", "--target", device, "--format", "s16",
                         "--rate", "48000", "--channels", "2", "--properties",
                         "stream.capture.sink=true" if device == "test_sink" else "{}", str(path))
        time.sleep(1.5)
        recorder.send_signal(signal.SIGINT)
        recorder.wait(timeout=5)
        with wave.open(str(path)) as audio:
            samples = array.array("h", audio.readframes(audio.getnframes()))[24000::2]
        samples = samples[:len(samples) // 48 * 48]
        assert len(samples) > 4800, "Too few audio samples"
        return tuple(2 * math.hypot(
            sum(v * math.cos(2 * math.pi * f * i / 48000) for i, v in enumerate(samples)),
            sum(v * math.sin(2 * math.pi * f * i / 48000) for i, v in enumerate(samples))) / len(samples)
                     for f in (1000, 2000))

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
        monitor = "ee_mic_monitor_output_level"
        tone(root / "mic.wav", 1000)
        tone(root / "desktop.wav", 2000)
        for sharing, includes_effects, volume in ((False, False, 1.0), (True, False, 1.0),
                                                  (True, False, 0.25), (True, True, 0.25)):
            # Seed each fixture only while Easy Effects is stopped.
            config.write_text(
                "[EffectsPipelines]\ninactivityTimeout=1\nprocessAllInputs=false\nprocessAllOutputs=false\n"
                "[StreamInputs]\ninputDevice=test_source\nuseDefaultInputDevice=false\n"
                f"listenToMic=true\nlistenToMicVolume={volume}\n"
                f"listenToMicIncludesOutputEffects={str(includes_effects).lower()}\n"
                "[StreamOutputs]\noutputDevice=test_sink\nuseDefaultOutputDevice=false\n"
                f"linkToVirtualSource={str(sharing).lower()}\n")
            server = start(binary, "--service-mode", "--hide-window")
            wait_for(lambda: (root / "runtime/EasyEffectsServer").exists(), "EasyEffects")
            target = "easyeffects_sink" if includes_effects else "test_sink"
            wait_for(lambda: has_link("ee_sie_output_level", monitor), "processed microphone monitor")
            wait_for(lambda: has_link(monitor, target), "monitor destination")
            assert not has_link("easyeffects_source", target), "Shared desktop audio reaches monitoring"
            # Monitoring keeps the input pipeline alive without recording clients.
            time.sleep(2)
            assert has_link("ee_sie_output_level", monitor), "Monitoring suspended by inactivity timer"
            if includes_effects:
                wait_for(lambda: has_link("ee_soe_output_level", "easyeffects_source"), "audio sharing")
            desktop = start("pw-cat", "--playback", "--target", "easyeffects_sink", str(root / "desktop.wav"))
            mic = start("pw-cat", "--playback", "--target", "0", "--properties",
                        "{ node.name=test_mic node.autoconnect=false }", str(root / "mic.wav"))
            wait_for(lambda: any(o.get("info", {}).get("props", {}).get("node.name") == "test_mic"
                                 for o in graph()), "microphone test signal")
            for channel in ("FL", "FR"):
                subprocess.run(["pw-link", f"test_mic:output_{channel}", f"test_source:input_{channel}"],
                               check=True, capture_output=True, timeout=5)
            time.sleep(0.5)
            mic_level, desktop_level = capture_amplitudes("test_sink")
            assert math.isclose(mic_level, 3276 * volume, rel_tol=0.03), (mic_level, volume)
            assert math.isclose(desktop_level, 3276, rel_tol=0.03), desktop_level
            if not includes_effects:
                recorded_mic, recorded_desktop = capture_amplitudes("easyeffects_source")
                assert math.isclose(recorded_mic, 3276, rel_tol=0.03), recorded_mic
                assert math.isclose(recorded_desktop, 3276, rel_tol=0.03) if sharing else recorded_desktop < 2
            for player in (mic, desktop):
                player.terminate()
                player.wait(timeout=5)
            assert cli("--quit").returncode == 0
            server.wait(timeout=10)
            wait_for(lambda: not has_link("ee_sie_output_level", monitor), "monitor cleanup")
            print(f"PASS: sharing={sharing}, output effects={includes_effects}, monitor volume={volume}", flush=True)
        print("PASS: isolated audio-sharing routing", flush=True)
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
        with tempfile.TemporaryDirectory(prefix="ee-routing-test-") as temporary:
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
