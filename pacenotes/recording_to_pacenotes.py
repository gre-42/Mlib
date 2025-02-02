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
            f'Recording "{args.recording}" does not have 7 columns')
    if (recording.shape[0] == 0):
        raise ValueError(f'Recording "{args.recording}" has zero rows')
    if args.trafo == 'geographic':
        trafo = latitude_longitude_2_meters_mapping(
            recording[0, 1], recording[0, 2])
        coords = trafo.transformed(recording[:, [1, 2]])
    else:
        coords = recording[:, [1, 3]]
    # periodic extension
    if args.circular:
        coords = np.concatenate([coords, coords], axis=0)
    if args.sigma != 0:
        coords = gaussian_filter1d(coords, args.sigma, axis=0)
    if args.pacenotes is not None:
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
                direction={-1: 'right', 1: 'left'}[sign],
                gear=int(gear)))
        with open(args.pacenotes, 'w') as f:
            json.dump(
                dict(frames=recording.shape[0],
                    length_in_meters=distances[recording.shape[0] - 1],
                    pacenotes=pacenotes),
                f,
                indent=4)

    if args.circular:
        coords = coords[:recording.shape[0], :]

    if (args.savefig is not None) or (args.map_ini is not None):
        pixels_per_meter = 0.5
        dpi = 300
        mins = np.min(coords, axis=0)
        maxs = np.max(coords, axis=0)
        size = maxs - mins
        print('size in meters', size, 'mins', mins)
        padding = 0.02
        if args.savefig is not None:
            import matplotlib.pyplot as plt
            figsize_inner = pixels_per_meter * size / dpi
            figsize_outer = 2 * padding + figsize_inner
            fig = plt.figure(
                figsize = figsize_outer,
                dpi = dpi)
            #plt.tight_layout()
            #plt.axis('off')
            ax = plt.Axes(fig, [0., 0., 1., 1.])
            ax.set_axis_off()
            fig.add_axes(ax)
            plt.plot(
                padding / figsize_outer[0] + figsize_inner[0] / figsize_outer[0] * (coords[:, 0] - mins[0]) / size[0],
                padding / figsize_outer[1] + figsize_inner[1] / figsize_outer[1] * (coords[:, 1] - mins[1]) / size[1],
                '-',
                color='white',
                linewidth=2,
                scalex=False,
                scaley=False)
            plt.plot(
                padding / figsize_outer[0] + figsize_inner[0] / figsize_outer[0] * (coords[:, 0] - mins[0]) / size[0],
                padding / figsize_outer[1] + figsize_inner[1] / figsize_outer[1] * (coords[:, 1] - mins[1]) / size[1],
                '-',
                color='black',
                linewidth=1,
                scalex=False,
                scaley=False)
            plt.gca().invert_yaxis()
            plt.savefig(
                args.savefig,
                bbox_inches='tight', 
                transparent=True,
                pad_inches=0,
                dpi=dpi)
        if args.map_ini is not None:
            with open(args.map_ini, 'w') as f:
                f.write(
                    f'[PARAMETERS]\n'
                    f'WIDTH={2 * padding * dpi + pixels_per_meter * size[0]}\n'
                    f'HEIGHT={2 * padding * dpi + pixels_per_meter * size[1]}\n'
                    f'MARGIN=20\n'
                    f'SCALE_FACTOR={1 / pixels_per_meter}\n'
                    f'X_OFFSET={padding * dpi / pixels_per_meter - mins[0]}\n'
                    f'Z_OFFSET={padding * dpi / pixels_per_meter - mins[1]}\n'
                    f'DRAWING_SIZE=10\n')

    if args.show:
        import matplotlib.pyplot as plt
        plt.plot(coords[:, 0], coords[:, 1], '--', color='green')
        # plt.plot(coords[changes, 0], coords[changes, 1], '-+')
        for p in pacenotes:
            plt.plot(
                coords[p['i0']:p['i1']+1, 0],
                coords[p['i0']:p['i1']+1, 1],
                color={'left': 'red', 'right': 'blue'}[p['direction']])
        ax = plt.gca()
        ax.set_aspect('equal', adjustable='box')
        a = 0
        b = 100
        plt.arrow(
            coords[a, 0], coords[a, 1],
            coords[b, 0] - coords[a, 0],
            coords[b, 1] - coords[a, 1],
            head_width=50, head_length=50, fc='k', ec='k')
        plt.show()


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('recording')
    parser.add_argument('pacenotes')
    parser.add_argument('--trafo', choices=['geographic', 'euclidean'], required=True)
    parser.add_argument('--sigma', type=float, default=0)
    parser.add_argument('--circular', action='store_true')
    parser.add_argument('--show', action='store_true')
    parser.add_argument('--savefig')
    parser.add_argument('--map_ini')

    args = parser.parse_args()

    run(args)
