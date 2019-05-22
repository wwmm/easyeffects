#!/usr/bin/env bash

pulseaudio -D --exit-idle-time=-1
pactl load-module module-null-sink sink_name=DummyOutput sink_properties=device.description="Virtual_Dummy_Output"
pactl load-module module-null-sink sink_name=MicOutput sink_properties=device.description="Virtual_Microphone_Output"
pacmd set-default-source MicOutput.monitor
pacmd set-default-sink DummyOutput
