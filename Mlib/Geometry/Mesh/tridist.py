#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

npoints = 3

A = np.array([0, 0])
B = np.array([1, 0])
C = np.array([1, 1])

res = []

npoints2 = npoints + 2
print('n', npoints * (npoints + 1) / 2)
for u in range(1, npoints2):
    for v in range(1, npoints2 - u):
        a = u / npoints2;
        b = v / npoints2;
        c = (npoints2 - u - v) / npoints2;
        pos = A * a + B * b + C * c;
        res.append(pos)

ar = np.array([A, B, C])
plt.scatter(ar[:, 0], ar[:, 1])

ar2 = np.array(res)
plt.scatter(ar2[:, 0], ar2[:, 1])

plt.show()
