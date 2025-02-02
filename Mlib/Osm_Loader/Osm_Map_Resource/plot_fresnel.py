#!/usr/bin/env python3

import numpy as np

import matplotlib.pyplot as plt


def main(min, max, exponent):
    a = np.linspace(0, np.pi / 2, 100)
    plt.plot(a * 180 / np.pi, min + (max - min) * ((1-np.cos(a)) ** exponent))
    plt.show()


if __name__ == '__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('--min', type=float, default=0)
    parser.add_argument('--max', type=float, default=1)
    parser.add_argument('--exponent', type=float, default=4)
    args = parser.parse_args()

    main(args.min, args.max, args.exponent)
