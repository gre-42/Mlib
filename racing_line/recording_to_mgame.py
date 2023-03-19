#!/usr/bin/env python3

from argparse import ArgumentParser

import numpy as np
from scipy.spatial.transform import Rotation as R


def geographic_coordinates_to_3d(lat, lon, height):
    '''
    Note: lat and lon are in radians.
    '''
    R = 6_371_000 + height
    r = R * np.cos(lat)
    x = r * np.cos(lon)
    y = r * np.sin(lon)
    z = R * np.sin(lat)
    return np.array([x, y, z])


def geographic_angle_to_3d(lat, lon, angle):
    '''
    Note: lat, lon and angle are in radians.
    '''
    # Find:
    # d/dh (f(lat + h * sin(a), lon + h * cos(a) * cos(lat)))

    # (%i13) f(lat, lon):=[cos(lat)*cos(lon), cos(lat)*sin(lon), sin(lat)];
    # (%o13) f(lat,lon):=[cos(lat)*cos(lon),cos(lat)*sin(lon),sin(lat)]
    # (%i14) diff(f(lat, lon), lat);
    # (%o14) [-sin(lat)*cos(lon),-sin(lat)*sin(lon),cos(lat)]
    # (%i15) diff(f(lat, lon), lon);
    # (%o15) [-cos(lat)*sin(lon),cos(lat)*cos(lon),0]
    df_lat = np.array([-np.sin(lat) * np.cos(lon),
                       -np.sin(lat) * np.sin(lon),
                       np.cos(lat)])
    df_lon = np.array([-np.cos(lat) * np.sin(lon),
                       np.cos(lat) * np.cos(lon),
                       np.zeros_like(lat)])
    v = np.sin(angle) * df_lat + np.cos(angle) * np.cos(lat) * df_lon
    return v / np.sqrt(np.sum(np.square(v), axis=0))


def euler_to_angle_y(angle, vec):
    dir = R.from_euler('zyx', angle).apply(vec)
    return np.arctan2(-dir[:, 2], dir[:, 0])


def run(args):
    recording = np.loadtxt(args.recording)
    if recording.ndim != 2:
        raise ValueError(
            f'Recording "{args.recording}" does not have 2 dimensions')
    if recording.shape[1] != 7:
        raise ValueError(
            f'Recording "{args.recording}" does not have 2 columns')
    # in: time, lat, lon, height, rx, ry, rz
    coords = geographic_coordinates_to_3d(recording[:, 1] * np.pi / 180,
                                          recording[:, 2] * np.pi / 180,
                                          recording[:, 3]).T
    dt = (recording[-1, 0] - recording[0, 0]) / recording.shape[0]
    angle_y_nz = euler_to_angle_y(recording[:, 4:7], [0, 0, -1])
    n = geographic_angle_to_3d(
        recording[:, 1] * np.pi / 180,
        recording[:, 2] * np.pi / 180,
        angle_y_nz[None, :]).T
    accel = np.sum(
        n[1:-1, :] *
        (
            coords[2:, :]
            - 2 * coords[1:-1, :]
            + coords[:-2]),
        axis=1) / (dt**2)
    accel = np.concatenate([[0.0], accel, [0.0]])

    # out: lat, lon, yangle, time, accel, brake
    np.savetxt(args.racing_line_mgame, np.column_stack([
        recording[:, 1],
        recording[:, 2],
        np.fmod(angle_y_nz + np.pi / 2, 2 * np.pi),
        recording[:, 0],
        args.mass * np.maximum(accel, 0.0),
        args.mass * np.minimum(accel, 0.0)])[::args.down_sampling])


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('recording')
    parser.add_argument('racing_line_mgame')
    parser.add_argument('--mass', type=float, required=True)
    parser.add_argument('--down_sampling', type=int, required=True)

    run(parser.parse_args())
