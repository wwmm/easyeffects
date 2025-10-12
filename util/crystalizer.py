#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile as wavefile

rate, wave = wavefile.read('test.wav')

wave_y = wave[14000:15000]
wave_x = np.arange(wave_y.size)

t = wave_x
original = wave_y

enhanced = np.copy(original)
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

for n in range(enhanced.size):
    enhanced[n] -= np.tanh(intensity * deriv2[n])

fig = plt.figure()

plt.plot(t, original, 'bo-', markersize=4, label='original')
plt.plot(t, enhanced, 'ro-', markersize=4, label='enhanced')

fig.legend()

plt.xlabel('Arbitrary Time', fontsize=18)
plt.ylabel('Waveform', fontsize=18)
plt.grid()

plt.show()
