import numpy as np
from matplotlib import pyplot as plt

from skimage import data, img_as_float
from skimage.feature import corner_harris, corner_peaks
from imageio import imread


def harris(image, **kwargs):
    return corner_peaks(corner_harris(image), **kwargs)

def plot_harris_points(image, filtered_coords):
    """ plots corners found in image"""

    plt.imshow(image)
    y, x = np.transpose(filtered_coords)
    plt.plot(x, y, 'b.')
    plt.axis('off')

# display results
plt.figure(figsize=(8, 3))
# im_lena = img_as_float(data.lena())
im_lena = np.mean(img_as_float(imread('G:/sfm_source_1024/1-video/pictures/video-010.png')), axis=2)
im_text = img_as_float(data.text())

filtered_coords = harris(im_lena, min_distance=5, threshold_rel=0.02)

plt.axes([0, 0, 0.3, 0.95])
plot_harris_points(im_lena, filtered_coords)

filtered_coords = harris(im_text, min_distance=4)

plt.axes([0.2, 0, 0.77, 1])
plot_harris_points(im_text, filtered_coords)

plt.show()
