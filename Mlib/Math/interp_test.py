#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np


def ninterp(p0, p1, n0, n1, t):
    d = np.linalg.norm(p0 - p1) / 2

    t0 = np.concatenate([[0], t, [1]])

    # From: https://math.stackexchange.com/a/3253471/233679
    k = 2
    a = (1 / t - 1) ** (-k)
    t1 = np.concatenate([[0], (1 - 1 / (1 + a)), [1]])

    return ((p0[None, :] + n0[None, :] * d * t0[:, None]) * (1-t1)[:, None] +
            (p1[None, :] - n1[None, :] * d * (1-t0)[:, None]) * t1[:, None])


def ninterpn(p, t):
    res = []
    for i in range(len(p) - 1):
        n0 = p[i + 1] - p[max(0, i - 1)]
        n1 = p[min(len(p) - 1, i + 2)] - p[i]
        n0 /= np.linalg.norm(n0)
        n1 /= np.linalg.norm(n1)
        t = np.linspace(0, 1, 10)[1 : -1]
        res.append(ninterp(p[i], p[i + 1], n0, n1, t))
    return np.concatenate(res, axis=0)


def run0():
    n0 = np.array([1., 2.])
    n1 = np.array([1., -2.])

    n0 /= np.linalg.norm(n0)
    n1 /= np.linalg.norm(n1)

    p0 = np.array([3, 4])
    p1 = np.array([6, 5])

    t = np.linspace(0, 1, 10)

    c = ninterp(p0, p1, n0, n1, t)
    plt.plot(c[:, 0], c[:, 1], '+-')
    plt.show()


def run():
    x = np.linspace(0, 2 * np.pi, 10)
    p = np.array([np.cos(x), np.sin(x)]).T
    # p = np.array([x, np.sin(x)]).T
    p_i = ninterpn(p, np.linspace(0, 1, 10))
    plt.plot(p_i[:, 0], p_i[:, 1], '-')
    plt.plot(p[:, 0], p[:, 1], '+')
    plt.axis('equal')
    plt.show()


if __name__ == '__main__':
    run()
