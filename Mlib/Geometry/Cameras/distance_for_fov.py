#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt


def circle_dist(m: float, r: float) -> float:
    return -(np.sqrt(m**2+1)*r)/m


r = 1.4

a = np.linspace(0, 2 * np.pi, 100)
x = np.cos(a) * r
y = np.sin(a) * r
m = -np.tan(60 * np.pi / 180 / 2)

if True:
    x1 = circle_dist(m, r)
if False:
    xs2 = m**2 * r**2 / (m**2 + 1)
    xs = np.sqrt(xs2)
    x1 = xs - np.sqrt(r**2-xs2)/m

t = - m * x1
xx = np.linspace(0, 3, 100)
yy = m * xx + t

plt.plot(x, y)
plt.plot(xx, yy)
# plt.plot([xs], [np.sqrt(r**2-xs**2)], '+')
plt.axis('square')
plt.show()
