#!/usr/bin/env python3

def _modify_path():
    import os.path
    import sys
    sys.path.append(os.path.dirname(
        os.path.dirname(os.path.abspath(__file__))))


_modify_path()

import json
from argparse import ArgumentParser

import numpy as np
from geography.geography import latitude_longitude_2_meters_mapping
from scipy.ndimage import gaussian_filter1d


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
    trafo = latitude_longitude_2_meters_mapping(
        recording[0, 1], recording[0, 2])
    coords = np.dot(trafo.R, recording[:, [1, 2]].T).T
    # periodic extension
    if args.circular:
        coords = np.concatenate([coords, coords], axis=0)
    coords = gaussian_filter1d(coords, args.sigma, axis=0)
    # From: https://en.wikipedia.org/wiki/Curvature#In_terms_of_a_general_parametrization
    d1 = np.gradient(coords, axis=0)
    d2 = np.gradient(d1, axis=0)
    x1, y1 = d1.T
    x2, y2 = d2.T
    k = (x1 * y2 - y1 * x2) / np.power(np.square(x1) + np.square(y1), 3 / 2)
    (changes,) = np.nonzero(np.sign(k[1:]) != np.sign(k[:-1]))
    changes += 1
    # undo periodic extension
    if args.circular:
        changes = changes[:changes.shape[0] // 2 + 1]
    distances = np.concatenate(
        [[0],
         np.cumsum(np.sqrt(np.sum(np.square(np.diff(coords, axis=0)),
                                  axis=1)))],
        axis=0)
    pacenotes = []
    for i0, i1 in zip(changes, changes[1:]):
        k_segment = k[i0:i1]
        sign = np.sign(np.mean(k_segment))
        v = 5 / np.sqrt(np.abs(k_segment))
        # print(v)
        gear = np.searchsorted([5, 50, 70, 100, 150, 200], np.min(v)) + 1
        pacenotes.append(dict(
            i0=int(i0),
            i1=int(i1),
            meters_to_start0=distances[i0],
            meters_to_start1=distances[i1],
            direction={1: 'right', -1: 'left'}[sign],
            gear=int(gear)))
    with open(args.pacenotes, 'w') as f:
        json.dump(
            dict(frames=recording.shape[0],
                 length_in_meters=distances[recording.shape[0] - 1],
                 pacenotes=pacenotes),
            f,
            indent=4)

    if args.plot:
        import matplotlib.pyplot as plt
        # plt.plot(coords[:, 0], coords[:, 1])
        # plt.plot(coords[changes, 0], coords[changes, 1], '-+')
        for p in pacenotes:
            plt.plot(
                coords[p['i0']:p['i1'], 0],
                coords[p['i0']:p['i1'], 1],
                color={'left': 'red', 'right': 'blue'}[p['direction']])
        plt.show()


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('recording')
    parser.add_argument('pacenotes')
    parser.add_argument('--sigma', type=float, default=0)
    parser.add_argument('--circular', action='store_true')
    parser.add_argument('--plot', action='store_true')

    args = parser.parse_args()

    run(args)
