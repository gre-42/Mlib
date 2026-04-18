#!/usr/bin/env python3

def _modify_path():
    import os.path
    import sys
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_modify_path()

from argparse import ArgumentParser

import numpy as np
from geography.geography import latitude_longitude_2_meters_mapping
from osm_reader import OsmReader


def run(args):
    osm = OsmReader(args.osm, args.only_raceways)

    start = args.start_node_id
    trafo = latitude_longitude_2_meters_mapping(
        *osm.coordinates[args.start_node_id])
    coords = []
    if args.circular:
        while osm.successors[start] != args.start_node_id:
            coords.append(osm.coordinates[start])
            start = osm.successors[start]
    else:
        while start is not None:
            coords.append(osm.coordinates[start])
            start = osm.successors.get(start, None)
    coords = np.array(coords)
    dir = np.gradient(trafo.transformed(coords), axis=0)
    yangles = np.arctan2(dir[:, 1], dir[:, 0])
    # out: lat, lon, yangle, time, accel, brake
    np.savetxt(
        args.recording,
        np.array([
            coords[:, 0],
            coords[:, 1],
            np.fmod(yangles + np.pi / 2, 2 * np.pi),
            np.full(yangles.shape, np.nan),
            np.full(yangles.shape, np.nan),
            np.full(yangles.shape, np.nan)]).T)
        

if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('osm')
    parser.add_argument('recording')
    parser.add_argument('--start_node_id', required=True)
    parser.add_argument('--only_raceways', action='store_true')
    parser.add_argument('--circular', action='store_true')

    run(parser.parse_args())
