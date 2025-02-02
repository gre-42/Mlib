#!/usr/bin/env python3

import os.path
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


def load_states(args):
    result = []
    with open(os.path.join(args.racing_line_raw,
                           'mintime', 'states.csv')) as f:
        reader = DictReader(
            filter(lambda row: row[0] != '#', f),
            fieldnames=['s_m', 't_s', 'v_mps', 'beta_rad', 'omega_z_radps',
                        'n_m', 'xi_rad'],
            delimiter=';')
        for l in reader:
            result.append((np.float64(l['s_m']),
                           np.float64(l['t_s']),
                           np.float64(l['v_mps']),
                           np.float64(l['beta_rad']),
                           np.float64(l['omega_z_radps']),
                           np.float64(l['n_m']),
                           np.float64(l['xi_rad'])))
    return np.array(result)


def s_interp(t, s):
    td = np.linspace(t[0], t[-1], len(t))
    plt.plot(td, interp1d(t, s, kind='cubic')(td))
    plt.title('s interpolated')
    plt.show()


def acceleration_s(t, s):
    td = np.linspace(t[0], t[-1], len(t))
    dt = (t[-1] - t[0]) / (len(t) - 1)
    v = np.diff(interp1d(t, s, kind='cubic')(td), n=1) / dt
    a = np.diff(interp1d(t, s, kind='cubic')(td), n=2) / dt**2
    # print('\n'.join(map(str, a)))
    print('s median', np.median(np.abs(a)))
    plt.plot(td[1:], v, label='v')
    plt.plot(td[2:], a, label='a')
    plt.legend()
    plt.title('s accel')
    plt.show()


def acceleration_vec(t, atot, x, y):
    td = np.linspace(t[0], t[-1], len(t))
    dt = (t[-1] - t[0]) / (len(t) - 1)
    dd1x = np.diff(interp1d(t, x, kind='cubic')(td), n=1) / dt
    dd1y = np.diff(interp1d(t, y, kind='cubic')(td), n=1) / dt
    v = np.sqrt(dd1x**2 + dd1y**2)
    dd2x = np.diff(interp1d(t, x, kind='cubic')(td), n=2) / dt**2
    dd2y = np.diff(interp1d(t, y, kind='cubic')(td), n=2) / dt**2
    a = np.sqrt(dd2x**2 + dd2y**2)
    # print('\n'.join(map(str, a)))
    print('vec median', np.median(a))
    dd1s = np.array([dd1x, dd1y])
    dir = dd1s / np.sqrt(np.sum(np.square(dd1s), axis=0))[None, :]
    a_n = np.sum(np.array([dd2x, dd2y]) * dir[:, :-1], axis=0)
    plt.plot(td[2:], a_n, label='$a_n$')
    plt.plot(td[2:], np.sqrt(np.square(a) - np.square(a_n)), label='$a_t$')
    # plt.plot(td[1:-1], dx)
    # plt.plot(td[1:-1], dy)
    plt.plot(td[1:], v, label='v')
    plt.plot(td[2:], a, label='a')
    plt.plot(t, atot, label='atot')
    plt.legend()
    plt.title('vec accel')
    plt.show()


def trajectory(x, y, sl):
    plt.plot(x, y)
    plt.plot(x[sl], y[sl], color='r')
    plt.axis('equal')
    plt.title('trajectory')
    plt.show()


def plot_states(states):
    s_m = states[:, 0]
    t_s = states[:, 1]
    v_mps = states[:, 2]
    beta_rad = states[:, 3]
    omega_z_radps = states[:, 4]
    n_m = states[:, 5]
    xi_rad = states[:, 6]
    plt.plot(s_m, n_m)
    plt.title('n')
    plt.show()


def run(args):
    location_s_m, location = load_location(args, sigma=0)
    accelera_s_m, accelera = load_accelerations(args)
    accelera_interp = np.array([
        np.interp(location_s_m, accelera_s_m, accelera[:, 0]),
        np.interp(location_s_m, accelera_s_m, accelera[:, 1]),
        np.interp(location_s_m, accelera_s_m, accelera[:, 2]),
        np.interp(location_s_m, accelera_s_m, accelera[:, 3])]).T
    states = load_states(args)
    sl = slice(args.begin, args.end)
    # x, y, t, ax, ay, atot
    # np.savetxt(args.racing_line_stats, np.hstack([location, accelera_interp]))
    s_interp(accelera_interp[sl, 0], location_s_m[sl])
    acceleration_vec(accelera_interp[sl, 0],
                     accelera_interp[sl, 3],
                     location[sl, 0],
                     location[sl, 1])
    acceleration_s(accelera[:, 0], accelera_s_m[:])
    trajectory(location[:, 0], location[:, 1], sl)
    plot_states(states)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('racing_line_raw')
    parser.add_argument('--begin', type=int)
    parser.add_argument('--end', type=int)

    run(parser.parse_args())
