#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile as wavefile

rate, wave = wavefile.read('test.wav')

wave_y = wave[14000:15000]
wave_x = np.arange(wave_y.size)

t = wave_x
original = wave_y

standard = np.copy(original)
deriv2 = np.zeros(original.size)

intensity = 2.0

# the boundaries are better handled in the plugin code

for n in range(original.size):
    if n > 0 and n < original.size - 1:
        deriv2[n] = original[n + 1] - 2 * original[n] + original[n - 1]
    elif n == 0:
        deriv2[0] = original[n + 1] - 2 * original[n] + original[n]
    elif n == original.size - 1:
        deriv2[n] = original[n] - 2 * original[n] + original[n - 1]

for n in range(standard.size):
    standard[n] -= intensity * deriv2[n]

aggressive = np.copy(standard)

if intensity >= 1:
    ndivs = 1000
    gain = np.linspace(1, intensity, ndivs)
    dv = 0.5 / ndivs

    for n in range(standard.size):
        v = aggressive[n]

        idx = int(np.floor(np.fabs(v) / dv))

        if idx < 0:
            idx = 0
        elif idx > gain.size:
            idx = gain.size - 1

        aggressive[n] = v * gain[idx]


fig = plt.figure()

plt.plot(t, original, 'bo-', markersize=4, label='original')
plt.plot(t, standard, 'ro-', markersize=4, label='standard')
plt.plot(t, aggressive, 'go-', markersize=4, label='aggressive')

fig.legend()

plt.xlabel('Arbitrary Time', fontsize=18)
plt.ylabel('Waveform', fontsize=18)
plt.grid()

plt.show()
