#!/usr/bin/env python3

def _modify_path():
    import os.path
    import sys
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_modify_path()

from argparse import ArgumentParser
from csv import DictWriter

import numpy as np
from geography.geography import latitude_longitude_2_meters_mapping
from osm_reader import OsmReader


def run(args):
    osm = OsmReader(args.osm, args.only_raceways)

    start = args.start_node_id
    trafo = latitude_longitude_2_meters_mapping(
        *osm.coordinates[args.start_node_id])
    np.savetxt(args.translation, trafo.t)
    np.savetxt(args.rotation, trafo.R)
    with open(args.track, 'w', newline='') as csvfile:
        writer = DictWriter(
            csvfile,
            fieldnames=[
                '# x_m', 'y_m', 'w_tr_right_m', 'w_tr_left_m'])
        writer.writeheader()
        while osm.successors[start] != args.start_node_id:
            coords = trafo.transformed(osm.coordinates[start])
            writer.writerow({
                '# x_m': str(coords[0]),
                'y_m': str(coords[1]),
                'w_tr_right_m': args.street_width / 2,
                'w_tr_left_m': args.street_width / 2})
            start = osm.successors[start]


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('osm')
    parser.add_argument('track')
    parser.add_argument('--translation', required=True)
    parser.add_argument('--rotation', required=True)
    parser.add_argument('--start_node_id', required=True)
    parser.add_argument('--street_width', type=float, required=True)
    parser.add_argument('--only_raceways', action='store_true')

    run(parser.parse_args())
