#!/usr/bin/env python3

from argparse import ArgumentParser
from csv import DictReader
import os

import numpy as np


def load_positions(args):
    t = np.loadtxt(args.translation)
    R = np.loadtxt(args.rotation)
    positions_s_m_list = []
    positions_list = []
    with open(os.path.join(args.racing_line_raw, 'traj_race_cl.csv')) as f:
        reader = DictReader(
            filter(lambda row: row[0]!='#', f),
            fieldnames=['s_m', 'x_m', 'y_m',
                        'psi_rad', 'kappa_radpm', 'vx_mps', 'ax_mps2'],
            delimiter=';')
        for l in reader:
            positions_s_m_list.append(np.float64(l['s_m']))
            positions_list.append((np.float64(l['x_m']),
                                   np.float64(l['y_m'])))
    return (np.array(positions_s_m_list),
            np.dot(-t + np.array(positions_list), np.linalg.inv(R.T)))


def load_controls(args):
    t = np.loadtxt(args.translation)
    R = np.loadtxt(args.rotation)
    controls_s_m_list = []
    controls_list = []
    with open(os.path.join(args.racing_line_raw,
                           'mintime', 'controls.csv')) as f:
        reader = DictReader(
            filter(lambda row: row[0]!='#', f),
            fieldnames=['s_m', 't_s', 'delta_rad', 'f_drive_N', 'f_brake_N',
                        'gamma_y_N'],
            delimiter=';')
        for l in reader:
            controls_s_m_list.append(np.float64(l['s_m']))
            controls_list.append((np.float64(l['f_drive_N']),
                                  np.float64(l['f_brake_N'])))
    return (np.array(controls_s_m_list),
            np.array(controls_list))


def run():
    parser = ArgumentParser()
    parser.add_argument('racing_line_raw')
    parser.add_argument('racing_line_mgame')
    parser.add_argument('--translation', required=True)
    parser.add_argument('--rotation', required=True)
    args = parser.parse_args()
    positions_s_m, positions = load_positions(args)
    controls_s_m, controls = load_controls(args)
    controls_interp = np.array([
        np.interp(positions_s_m, controls_s_m, controls[:, 0]),
        np.interp(positions_s_m, controls_s_m, controls[:, 1])]).T
    np.savetxt(args.racing_line_mgame, np.hstack([positions, controls_interp]))


if __name__ == '__main__':
    run()
