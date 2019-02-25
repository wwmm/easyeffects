#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile as wavefile

rate, wave = wavefile.read('test.wav')

wave_y = wave[0:500]
wave_x = np.arange(wave_y.size)

# print(rate)
# print(wave_x)
# print(wave_y)

t = wave_x
original = wave_y

ffmpeg = np.copy(original)
our = np.copy(original)

intensity = 4.0
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

# plt.xlabel('t [ s ]', fontsize=18)
# plt.ylabel('y [ m ]', fontsize=18)
plt.grid()

plt.show()
