#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d

f1, f2 = 50.0, 60.0  # frequencias

t1, t2 = 0.0, 100.0

# frequencia angular
omega1, omega2 = 2.0 * np.pi * f1, 2.0 * np.pi * f2


def y1(t): return np.cos(omega1 * t)


def y2(t): return np.cos(omega2 * t)


def y(t): return y1(t) + y2(t)


t = np.linspace(t1, t2, 1000)
original = y(t)

# interp_f = interp1d(t, original, kind='cubic')
# t = np.linspace(t1, t2, 200)
# original = interp_f(t)

ffmpeg = np.copy(original)
other = np.copy(original)

intensity = 10.0
last_v_ffmpeg = ffmpeg[0]
last_v_other = other[0]

for n in range(ffmpeg.size):
    # ffmpeg method

    v = ffmpeg[n]

    ffmpeg[n] = v + (v - last_v_ffmpeg) * intensity

    last_v_ffmpeg = v

    # other

    v = other[n]

    other[n] = last_v_other + (v - last_v_other) * intensity

    last_v_other = v

fig = plt.figure()

plt.plot(t, original, 'bo-', markersize=4, label='original')
plt.plot(t, ffmpeg, 'ro-', markersize=4, label='ffmpeg')
plt.plot(t, other, 'go-', markersize=4, label='other')

fig.legend()

plt.xlabel('t [ s ]', fontsize=18)
plt.ylabel('y [ m ]', fontsize=18)
plt.grid()

plt.show()
