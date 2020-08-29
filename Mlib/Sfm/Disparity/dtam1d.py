#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

def PI(x):
    return np.clip(x, 0, 1)

def A(x):
    return np.gradient(x)

def AT(x):
    return -np.gradient(x)

N = 100

plot = False
best_coeffs = None
best_var = np.inf

for σq in np.logspace(-3, 3, 20):
    for σd in np.logspace(-3, 3, 20):
        for ε in np.logspace(-3, 3, 10):

            #σq = 0.001
            #σd = 0.001
            #ε = 0.01
            q = np.zeros(N)
            G = np.ones(N)
            d = np.random.random(N)
            a = d.copy()
            θ = np.inf

            if plot:
                plt.plot(d)

            for i in range(1000):
                q = PI((q + σq * G * A(d)) / (1 + σq * ε))
                d = (d + σd * (-G * AT(q) + 1 / θ * a)) / (1 + σd / θ)

            print(np.var(d), np.var(np.random.random(N)), σq, σd, ε)
            if np.var(d) < best_var:
                best_var = np.var(d)
                best_coeffs = [σq, σd, ε]

            if plot:
                plt.plot(d, '+-')

                plt.show()

print(best_var)
print(best_coeffs)
