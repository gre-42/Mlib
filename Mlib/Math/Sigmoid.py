#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

def sigmoid(x: np.ndarray, t: float, k: float) -> np.ndarray:
    a = np.power(np.power(x, np.log(2) / np.log(t)) - 1, k)
    return 1 / (1 + a)


def main():
    x = np.linspace(0.5, 1, 100)
    plt.plot(x, sigmoid(x, 0.9, 4))
    plt.show()


if __name__ == '__main__':
    main()
