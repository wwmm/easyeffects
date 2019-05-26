#!/usr/bin/env bash

while [ -z "`pactl list sinks | awk '/PulseEffects_apps.monitor/{print}'`" ]
do
  sleep 1
  echo "waiting PulseEffects initialization"
done

echo "PulseEffects has been initialized"
