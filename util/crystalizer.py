#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

v = 344.0  # velocidade do som no ar

f1, f2 = 50.0, 60.0  # frequencias
A1, A2 = 1.0, 1.0     # amplitudes

x = 100.0
t1, t2 = 0.0, 200.0

# comprimento de onda
L1, L2 = v / f1, v / f2

# frequencia angular
omega1, omega2 = 2.0 * np.pi * f1, 2.0 * np.pi * f2

# numero de onda
k1, k2 = 2.0 * np.pi / L1, 2.0 * np.pi / L2


def y1(x, t): return A1 * np.cos(k1 * x - omega1 * t)


def y2(x, t): return A2 * np.cos(k2 * x - omega2 * t)


def y(x, t): return y1(x, t) + y2(x, t)


t = np.linspace(t1, t2, 1000)
original = y(x, t)
modified = np.copy(original)

intensity = 20.0
last_v = modified[0]

for n in range(modified.size):
    v = modified[n]

    print("before: ", v)

    modified[n] = v + (v - last_v) * intensity

    print("after: ", v)

    last_v = v

fig = plt.figure()

p1 = plt.plot(t, original, label='original')
p2 = plt.plot(t, modified, label='modified')

fig.legend()

plt.xlabel('t [ s ]', fontsize=18)
plt.ylabel('y [ m ]', fontsize=18)
plt.grid()

plt.show()
