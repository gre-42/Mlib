#!/usr/bin/env python3

from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument('source')
parser.add_argument('destination')

args = parser.parse_args()

import cv2 as cv
img = cv.imread(args.source)
if img is None:
    raise ValueError(f'Could not load {args.source}')

gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)

sift = cv.xfeatures2d.SIFT_create()
kp = sift.detect(gray, None)
img = cv.drawKeypoints(gray, kp, img)
cv.imwrite(args.destination, img)
