#!/usr/bin/env python3

from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument('source0')
parser.add_argument('source1')
parser.add_argument('destination')

args = parser.parse_args()

import cv2 as cv
img0 = cv.imread(args.source0,0)
img1 = cv.imread(args.source1,0)
# Initiate ORB detector
orb = cv.ORB_create(nfeatures=1000)

# find the keypoints with ORB
kp0 = orb.detect(img0,None)
kp1 = orb.detect(img1,None)

# compute the descriptors with ORB
kp0, des0 = orb.compute(img0, kp0)
kp1, des1 = orb.compute(img1, kp1)

# draw only keypoints location, not size and orientation
# img2 = cv.drawKeypoints(img, kp, None, color=(0,255,0), flags=0)

# img2 = cv.drawMatchesKnn(img0,kp0,img1,kp1,good,None,flags=cv.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS)

# BFMatcher with default params
bf = cv.BFMatcher()
matches = bf.knnMatch(des0,des1,k=2)

# Apply ratio test
good = []
for m,n in matches:
    if m.distance < 0.75*n.distance:
        good.append([m])
# cv.drawMatchesKnn expects list of lists as matches.
img2 = cv.drawMatchesKnn(img0,kp0,img1,kp1,good,None,flags=cv.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS)
cv.imwrite(args.destination, img2)
