#!/usr/bin/env python3

def _modify_path():
    import os.path
    import sys
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_modify_path()

from argparse import ArgumentParser

import numpy as np
from geography.geography import latitude_longitude_2_meters_mapping


def run(args):
    # columns: time, lat, lon, height, rx, ry, rz
    recording = np.loadtxt(args.recording)
    if recording.ndim != 2:
        raise ValueError(
            f'Recording "{args.recording}" does not have 2 dimensions')
    if recording.shape[1] != 7:
        raise ValueError(
            f'Recording "{args.recording}" does not have 2 columns')
    if (recording.shape[0] == 0):
        raise ValueError(f'Recording "{args.recording}" has zero rows')
    trafo = latitude_longitude_2_meters_mapping(recording[0, 1], recording[0, 2])
    coords = np.dot(trafo.R, recording[:, [1, 2]].T).T
    # From: https://en.wikipedia.org/wiki/Curvature#In_terms_of_a_general_parametrization
    d1 = np.gradient(coords, axis=0)
    d2 = np.gradient(d1, axis=0)
    x1, y1 = d1.T
    x2, y2 = d2.T
    k = (x1 * y2 - y1 * x2) / np.power(np.square(x1) + np.square(y1), 3 / 2)
    (changes,) = np.nonzero(np.sign(k[1:]) != np.sign(k[:-1]))
    changes += 1
    import matplotlib.pyplot as plt
    plt.plot(coords[:, 0], coords[:, 1])
    plt.plot(coords[changes, 0], coords[changes, 1], '-+')
    plt.show()

if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('recording')

    args = parser.parse_args()

    run(args)
