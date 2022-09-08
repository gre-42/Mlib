#!/usr/bin/env python3

def _modify_path():
    import os.path
    import sys
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_modify_path()

import xml.etree.ElementTree as ET
from argparse import ArgumentParser
from csv import DictWriter
from itertools import tee

import numpy as np
from geography.geography import latitude_longitude_2_meters_mapping


def pairwise(iterable):
    '''
    From: https://stackoverflow.com/a/5764807/2292832

    s -> (s0,s1), (s1,s2), (s2, s3), ...
    '''
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)


def run():
    parser = ArgumentParser()
    parser.add_argument('osm')
    parser.add_argument('track')
    parser.add_argument('--translation', required=True)
    parser.add_argument('--rotation', required=True)
    parser.add_argument('--start_node_id', required=True)
    parser.add_argument('--street_width', type=float, required=True)

    args = parser.parse_args()

    tree = ET.parse(args.osm)
    root = tree.getroot()
    coordinates = {
        node.attrib['id']: np.array((node.attrib['lat'], node.attrib['lon']),
                                    dtype=float)
        for node in root.iter('node')}
    successors_list = [
        (n0.attrib['ref'], n1.attrib['ref']) for way in root.iter(
            'way') for n0, n1 in pairwise(way.iter('nd'))]
    if (len(set(n for n, _ in successors_list)) != len(successors_list)):
        raise ValueError('Found nodes with more than one successor')
    successors = dict(successors_list)
    start = args.start_node_id
    trafo = latitude_longitude_2_meters_mapping(
        *coordinates[args.start_node_id])
    np.savetxt(args.translation, trafo.t)
    np.savetxt(args.rotation, trafo.R)
    with open(args.track, 'w', newline='') as csvfile:
        writer = DictWriter(
            csvfile,
            fieldnames=[
                '# x_m', 'y_m', 'w_tr_right_m', 'w_tr_left_m'])
        writer.writeheader()
        while successors[start] != args.start_node_id:
            coords = trafo.transformed(coordinates[start])
            writer.writerow({
                '# x_m': str(coords[0]),
                'y_m': str(coords[1]),
                'w_tr_right_m': args.street_width / 2,
                'w_tr_left_m': args.street_width / 2})
            start = successors[start]


if __name__ == '__main__':
    run()
