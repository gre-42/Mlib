#!/usr/bin/env python3

import xml.etree.ElementTree as ET
from argparse import ArgumentParser
from csv import DictWriter
from itertools import tee

import numpy as np

kilo = 1e3
meters = 1
degrees = np.pi / 180
radians = 1


def latitude_longitude_2_meters(
        latitude: float,
        longitude: float,
        latitude0: float,
        longitude0: float) -> np.ndarray:
    r0 = 6_371 * kilo * meters
    r1 = r0 * np.cos(latitude0 * degrees / radians)
    circumference0 = r0 * 2 * np.pi
    circumference1 = r1 * 2 * np.pi
    return np.array((
        (circumference1 / 360) * (longitude - longitude0),
        (circumference0 / 360) * (latitude - latitude0)),
        dtype=float)


class TransformationMatrix:
    t: np.ndarray
    R: np.ndarray

    def transformed(self, a: np.ndarray):
        return self.t + np.dot(self.R, a)


def latitude_longitude_2_meters_mapping(
        latitude0: float,
        longitude0: float) -> np.ndarray:
    '''
    Compute a transformation matrix that maps geographic coordinates to meters.

    Wrapper around latitude_longitude_2_meters (multiply by zeros and the identity matrix)
    to get a transformation matrix.
    '''
    result = TransformationMatrix()
    result.t = latitude_longitude_2_meters(0, 0, latitude0, longitude0)
    result.R = np.empty(shape=(2, 2))
    result.R[:, 0] = latitude_longitude_2_meters(
        1, 0, latitude0, longitude0) - result.t
    result.R[:, 1] = latitude_longitude_2_meters(
        0, 1, latitude0, longitude0) - result.t
    return result


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
        node.attrib['id']: np.array(
            (node.attrib['lat'], node.attrib['lon']), dtype=float)
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
