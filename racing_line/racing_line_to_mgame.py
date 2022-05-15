#!/usr/bin/env python3

from argparse import ArgumentParser
from csv import DictReader
import os

import numpy as np


def run():
    parser = ArgumentParser()
    parser.add_argument('racing_line_raw')
    parser.add_argument('racing_line_mgame')
    parser.add_argument('--translation', required=True)
    parser.add_argument('--rotation', required=True)
    args = parser.parse_args()

    t = np.loadtxt(args.translation)
    R = np.loadtxt(args.rotation)
    positions_list = []
    with open(os.path.join(args.racing_line_raw, 'traj_race_cl.csv')) as f:
        reader = DictReader(
            filter(lambda row: row[0]!='#', f),
            fieldnames=['s_m', 'x_m', 'y_m',
                        'psi_rad', 'kappa_radpm', 'vx_mps', 'ax_mps2'],
            delimiter=';')
        for l in reader:
            positions_list.append((np.float64(l['x_m']), np.float64(l['y_m'])))
    positions = t + np.dot(np.array(positions_list), np.linalg.inv(R.T))
    np.savetxt(args.racing_line_mgame, positions)


if __name__ == '__main__':
    run()
