#!/usr/bin/env python3

from PyAstronomy import pyasl
import matplotlib.pylab as plt
import datetime
import numpy as np

degrees = np.pi / 180

# Convert calendar date to JD
# use the datetime package
jd = datetime.datetime(2013, 4, 16, hour=12)
jd = pyasl.jdcnv(jd)

time, sun_ra, sun_dec = pyasl.sunpos(jd)

# x = np.arange(512)
# y = np.arange(256)
# xv, yv = np.meshgrid(x, y)
# 
# observer_lon = xv * 2 * np.pi / xv.shape[1] - np.pi
# observer_lat = yv * np.pi / yv.shape[0] - np.pi / 2
# observer_alt = np.zeros_like(xv)
# alt, az, ha = pyasl.eq2hor(
#     np.full(xv.flatten().shape, jd),
#     np.full(xv.flatten().shape, sun_ra),
#     np.full(xv.flatten().shape, sun_dec),
#     lon=observer_lon.flatten(),
#     lat=observer_lat.flatten(),
#     alt=observer_alt.flatten())

a = np.empty(shape=(256 // 10, 512 // 10))
for r in range(a.shape[0]):
    for c in range(a.shape[1]):
        observer_lon = c * 2 * np.pi / a.shape[1] - np.pi
        observer_lat = r * np.pi / a.shape[0] - np.pi / 2
        observer_alt = 2635
        alt, az, ha = pyasl.eq2hor(jd, sun_ra, sun_dec, lon=observer_lon / degrees, lat=observer_lat / degrees, alt=observer_alt)
        a[a.shape[0] - 1 - r, c] = np.maximum(0, -np.sin(alt * degrees))

plt.imshow(a, cmap=plt.cm.binary)
plt.show()
