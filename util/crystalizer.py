#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

f1, f2 = 50.0, 60.0  # frequencias

t1, t2 = 0.0, 200.0

# frequencia angular
omega1, omega2 = 2.0 * np.pi * f1, 2.0 * np.pi * f2


def y1(t): return np.cos(omega1 * t)


def y2(t): return np.cos(omega2 * t)


def y(t): return y1(t) + y2(t)


t = np.linspace(t1, t2, 1000)
original = y(t)

ffmpeg = np.copy(original)
our = np.copy(original)

intensity = 50.0 * 10
last_v_ffmpeg = ffmpeg[0]

for n in range(ffmpeg.size):
    v = ffmpeg[n]

    ffmpeg[n] = v + (v - last_v_ffmpeg) * intensity

    last_v_ffmpeg = v


last_v_our = our[-1]
for n in range(our.size - 1, 0, -1):
    v = our[n]
    our[n] = v + (v - last_v_our) * intensity
    last_v_our = v

our = 0.5 * (our + ffmpeg)

our[0] = our[1] + (our[1] - our[2])
our[-1] = our[-2] + (our[-2] - our[-3])

fig = plt.figure()

plt.plot(t, original, 'bo-', markersize=4, label='original')
# plt.plot(t, ffmpeg, 'go-', markersize=4, label='ffmpeg')
plt.plot(t, our, 'ro-', markersize=4, label='our')

fig.legend()

plt.xlabel('t [ s ]', fontsize=18)
plt.ylabel('y [ m ]', fontsize=18)
plt.grid()

plt.show()
