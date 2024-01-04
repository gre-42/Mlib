#!/usr/bin/env python3

import numpy as np

import matplotlib.pyplot as plt

a=np.linspace(0, np.pi / 2, 100)
plt.plot(a, (1-np.cos(a)) ** 4)
plt.show()
