#!/usr/bin/env python3

import os.path
from argparse import ArgumentParser
from csv import DictReader, DictWriter


def run(args):
    with open(os.path.join(args.racing_line_raw, 'traj_race_cl.csv')) as f_in:
        reader = DictReader(
            filter(lambda row: row[0] != '#', f_in),
            fieldnames=['s_m', 'x_m', 'y_m',
                        'psi_rad', 'kappa_radpm', 'vx_mps', 'ax_mps2'],
            delimiter=';')
        with open(args.race_track, 'w') as f_out:
            writer = DictWriter(
                f_out,
                fieldnames=[
                    '# x_m', 'y_m', 'w_tr_right_m', 'w_tr_left_m'])
            writer.writeheader()
            i = 0
            for l in reader:
                if i % args.down_sampling != 0:
                    writer.writerow({
                        '# x_m': l['x_m'],
                        'y_m': l['y_m'],
                        'w_tr_right_m': args.street_width / 2,
                        'w_tr_left_m': args.street_width / 2})
                i += 1


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('racing_line_raw')
    parser.add_argument('race_track')
    parser.add_argument('--down_sampling', type=int, required=True)
    parser.add_argument('--street_width', type=float, required=True)

    run(parser.parse_args())
