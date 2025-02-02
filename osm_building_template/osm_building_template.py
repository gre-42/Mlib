#!/usr/bin/env python3

def _modify_path():
    import os.path
    import sys
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_modify_path()

import os.path
import xml.etree.ElementTree as ET
from argparse import ArgumentParser

import numpy as np
import pywavefront
from geography.geography import (kilo, latitude_longitude_2_meters_mapping,
                                 meters)

parser = ArgumentParser()
parser.add_argument('obj_files', nargs='+')
parser.add_argument('--osm_file', required=True)
parser.add_argument('--latitude0', type=np.float64, required=True)
parser.add_argument('--longitude0', type=np.float64, required=True)
parser.add_argument('--boundary_size', type=float, default=10 * kilo * meters)


def _bbox(scene):
    return np.array([
        [min(v[0] for v in scene.vertices),
         min(v[1] for v in scene.vertices),
         min(v[2] for v in scene.vertices)],
        [max(v[0] for v in scene.vertices),
         max(v[1] for v in scene.vertices),
         max(v[2] for v in scene.vertices)]])


def _to_str(a):
    return str(a)


def _generate_node(parent, tag, id, coords):
    return ET.SubElement(parent, tag, id=id, lat=_to_str(coords[0]), lon=_to_str(coords[1]))
    

def run(args):
    osm = ET.Element('osm', version='0.6')
    l2m = latitude_longitude_2_meters_mapping(
        args.latitude0,
        args.longitude0)
    m2l = l2m.inverse
    minlat, minlon = m2l.transformed(np.array([-args.boundary_size, -args.boundary_size]))
    maxlat, maxlon = m2l.transformed(np.array([args.boundary_size, args.boundary_size]))
    ET.SubElement(
        osm,
        'bounds',
        minlat=_to_str(minlat),
        minlon=_to_str(minlon),
        maxlat=_to_str(maxlat),
        maxlon=_to_str(maxlon))
    element_id = -1
    offset = np.array([0., 0.])
    ways = []
    for f in args.obj_files:
        model_name = os.path.basename(f.replace('.obj', ''))
        way = []
        scene = pywavefront.Wavefront(f)
        bbox = _bbox(scene)
        offset[0] -= bbox[0][0]
        _generate_node(osm, 'node', id=_to_str(element_id), coords=(m2l.transformed(
            offset + [bbox[0][0], -bbox[1][2]])))
        way.append(element_id)
        element_id -= 1
        model_node = _generate_node(
            osm,
            'node',
            id=_to_str(element_id),
            coords=m2l.transformed(offset))
        ET.SubElement(
            model_node,
            'tag',
            k='model',
            v=model_name)
        way.append(element_id)
        element_id -= 1
        _generate_node(osm, 'node', id=_to_str(element_id), coords=(m2l.transformed(
            offset + [bbox[1][0], -bbox[1][2]])))
        way.append(element_id)
        element_id -= 1
        _generate_node(osm, 'node', id=_to_str(element_id), coords=(m2l.transformed(
            offset + [bbox[1][0], -bbox[0][2]])))
        way.append(element_id)
        element_id -= 1
        _generate_node(osm, 'node', id=_to_str(element_id), coords=(m2l.transformed(
            offset + [bbox[0][0], -bbox[0][2]])))
        way.append(element_id)
        element_id -= 1
        offset[0] += bbox[1][0] + 2
        ways.append((model_name, way))
    for model_name, way in ways:
        way_element = ET.SubElement(osm, 'way', id=_to_str(element_id))
        ET.SubElement(way_element, 'tag', k='name', v=model_name)
        for node in reversed(way):
            ET.SubElement(way_element, 'nd', ref=_to_str(node))
        ET.SubElement(way_element, 'nd', ref=_to_str(way[-1]))
        element_id -= 1
    with open(args.osm_file, 'wb') as f:
        tree = ET.ElementTree(osm)
        ET.indent(tree, space=" ", level=0)
        tree.write(f, encoding='utf-8', xml_declaration=True)


if __name__ == '__main__':
    run(parser.parse_args())
