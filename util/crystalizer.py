#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile as wavefile

rate, wave = wavefile.read('test.wav')

wave_y = wave[10000:11000]
wave_x = np.arange(wave_y.size)

t = wave_x
original = wave_y

processed = np.copy(original)

intensity = 4.0
last_v = processed[0]

for n in range(processed.size):
    v = processed[n]

    v1 = v + (v - last_v) * intensity

    v2 = 0.0
    if n < processed.size - 1:
        v2 = v + (v - processed[n + 1]) * intensity

    processed[n] = 0.5 * (v1 + v2)

    last_v = v


fig = plt.figure()

plt.plot(t, original, 'bo-', markersize=4, label='original')
plt.plot(t, processed, 'ro-', markersize=4, label='processed')

fig.legend()

plt.xlabel('Arbitrary Time', fontsize=18)
plt.ylabel('Waveform', fontsize=18)
plt.grid()

plt.show()
