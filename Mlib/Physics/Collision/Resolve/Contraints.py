#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

def simulate_shock_absorber(dt, n):
    t = np.arange(n) * dt
    x = np.empty(t.shape)

    v = 0
    distance = 0
    Ks = 1e5
    Ka = 1.5e4
    a0 = -9.8
    m = 1e3

    for i in range(len(x)):
        F = -(Ks * distance + Ka * v)
        v += dt * (F / m + a0)
        distance += dt * v
        x[i] = distance
    
    return t, x

for dt in [1 / 60, 1 / 60 / 10, 1 / 60 / 100]:
    t, x = simulate_shock_absorber(dt, round(2 / dt))
    plt.plot(t, x)

plt.show()
