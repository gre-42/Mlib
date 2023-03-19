import xml.etree.ElementTree as ET
from itertools import tee
from typing import Dict, List

import numpy as np


def pairwise(iterable):
    '''
    From: https://stackoverflow.com/a/5764807/2292832

    s -> (s0,s1), (s1,s2), (s2, s3), ...
    '''
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)


class OsmReader:
    coordinates: Dict[str, np.ndarray]
    successors: Dict[str, str]

    def __init__(self, filename: str, only_raceways: bool) -> None:
        tree = ET.parse(filename)
        root = tree.getroot()
        self.coordinates = {
            node.attrib['id']: np.array((node.attrib['lat'], node.attrib['lon']),
                                        dtype=float)
            for node in root.iter('node')}
        ways = (way for way in root.iter('way')
                if ((not only_raceways) or
                    way.attrib.get('raceway', 'no') == 'yes'))
        successors_list = [
            (n0.attrib['ref'], n1.attrib['ref'])
            for way in ways
            for n0, n1 in pairwise(way.iter('nd'))]
        if (len(set(n for n, _ in successors_list)) != len(successors_list)):
            raise ValueError('Found nodes with more than one successor')
        self.successors = dict(successors_list)
