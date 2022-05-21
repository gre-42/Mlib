#!/usr/bin/env python3

import os
from argparse import ArgumentParser
from csv import DictReader

import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import interp1d
from scipy.ndimage import gaussian_filter1d


def load_location(args, sigma=0):
    positions_s_m_list = []
    positions_list = []
    with open(os.path.join(args.racing_line_raw, 'traj_race_cl.csv')) as f:
        reader = DictReader(
            filter(lambda row: row[0] != '#', f),
            fieldnames=['s_m', 'x_m', 'y_m',
                        'psi_rad', 'kappa_radpm', 'vx_mps', 'ax_mps2'],
            delimiter=';')
        for l in reader:
            positions_s_m_list.append(np.float64(l['s_m']))
            positions_list.append((np.float64(l['x_m']),
                                   np.float64(l['y_m'])))
    if sigma != 0:
        positions_list = gaussian_filter1d(positions_list, sigma=sigma, axis=0)
    return (np.array(positions_s_m_list),
            np.array(positions_list))


def load_accelerations(args):
    accelera_s_m_list = []
    accelera_list = []
    with open(os.path.join(args.racing_line_raw,
                           'mintime', 'accelerations.csv')) as f:
        reader = DictReader(
            filter(lambda row: row[0] != '#', f),
            fieldnames=['s_m', 't_s', 'ax_mps2', 'ay_mps2', 'atot_mps2'],
            delimiter=';')
        for l in reader:
            accelera_s_m_list.append(np.float64(l['s_m']))
            accelera_list.append((np.float64(l['t_s']),
                                  np.float64(l['ax_mps2']),
                                  np.float64(l['ay_mps2']),
                                  np.float64(l['atot_mps2'])))
    return (np.array(accelera_s_m_list),
            np.array(accelera_list))


def s_interp(t, s):
    td = np.linspace(t[0], t[-1], len(t))
    plt.plot(td, interp1d(t, s, kind='cubic')(td))
    plt.show()


def acceleration_s(t, s):
    td = np.linspace(t[0], t[-1], len(t))
    dt = (t[-1] - t[0]) / (len(t) - 1)
    a = np.diff(interp1d(t, s, kind='cubic')(td), n=2) / dt**2
    # print('\n'.join(map(str, a)))
    print('median', np.median(np.abs(a)))
    plt.plot(td[1:-1], a)
    plt.show()


def acceleration_vec(t, x, y):
    td = np.linspace(t[0], t[-1], len(t))
    dt = (t[-1] - t[0]) / (len(t) - 1)
    ddx = np.diff(interp1d(t, x, kind='cubic')(td), n=2)
    ddy = np.diff(interp1d(t, y, kind='cubic')(td), n=2)
    a = np.sqrt(ddx**2 + ddy**2) / dt**2
    # print('\n'.join(map(str, a)))
    print('median', np.median(a))
    # plt.plot(td[1:-1], dx)
    # plt.plot(td[1:-1], dy)
    plt.plot(td[1:-1], a)
    plt.show()


def trajectory(x, y):
    plt.plot(x, y)
    plt.plot(x[3300:3400], y[3300:3400], color='r')
    plt.axis('equal')
    plt.show()


def run():
    parser = ArgumentParser()
    parser.add_argument('racing_line_raw')
    args = parser.parse_args()
    location_s_m, location = load_location(args, sigma=0)
    accelera_s_m, accelera = load_accelerations(args)
    accelera_interp = np.array([
        np.interp(location_s_m, accelera_s_m, accelera[:, 0]),
        np.interp(location_s_m, accelera_s_m, accelera[:, 1]),
        np.interp(location_s_m, accelera_s_m, accelera[:, 2]),
        np.interp(location_s_m, accelera_s_m, accelera[:, 3])]).T
    # x, y, t, ax, ay, atot
    # np.savetxt(args.racing_line_stats, np.hstack([location, accelera_interp]))
    s_interp(accelera_interp[3300:3400, 0], location_s_m[3300:3400])
    acceleration_vec(accelera_interp[3300:3400, 0],
                     location[3300:3400, 0], location[3300:3400, 1])
    acceleration_s(accelera[:100, 0], accelera_s_m[:100])
    trajectory(location[:, 0], location[:, 1])


if __name__ == '__main__':
    run()
