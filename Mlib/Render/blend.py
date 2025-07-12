#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
from argparse import ArgumentParser
from scipy.ndimage import gaussian_filter, convolve1d


def nfilt(image, n):
    if n == 0:
        return image.copy()
    kernel = [1/3] + (n-1) * [0] + [1/3] + (n-1) * [0] + [1/3]
    image = convolve1d(image, kernel, axis=0)
    image = convolve1d(image, kernel, axis=1)
    return image


def run(args):
    bg = plt.imread(args.background)
    fg = plt.imread(args.foreground)
    if bg.ndim != 3:
        raise ValueError('Unexpected background dimensionality')
    if fg.ndim != 3:
        raise ValueError('Unexpected foreground dimensionality')
    if bg.shape[2] != 3:
        raise ValueError('Background does not have 3 channels')
    if fg.shape[2] != 4:
        raise ValueError('Foreground does not have 4 channels')

    bg = bg.astype(float) / 255
    fg = fg.astype(float)
    alpha0 = fg[..., 3]
    fg = fg[..., :3]
    bg1 = gaussian_filter(bg, args.std, axes=[0, 1])
    brightness = np.mean(bg1, axis=2)
    if False:
        alpha = np.ones_like(alpha0)
        for i in range(10):
            alpha1 = gaussian_filter(alpha0, args.std * i / 20)
            alpha1 = np.clip(alpha1 * args.scale + args.offset, 0, 1)
            alpha2 = alpha0 * (1 - brightness) + alpha1 * brightness
            alpha2 = np.clip(alpha2 * args.scale + args.offset, 0, 1)
            alpha2 = np.minimum(alpha2, alpha0)
            # alpha0 = np.clip((alpha0 - args.offset) / args.scale, 0, 1)
            alpha = np.minimum(alpha, alpha2)
        alpha = alpha[..., None]
        alpha0 = alpha0[..., None]
        fg1 = bg1 * (1 - alpha) + fg * alpha
        blended = bg * (1 - alpha0) + fg1 * alpha0
        blended = np.maximum(blended, fg)
    if True:
        threshold = 0
        intensity = 0.5
        alpha0 = alpha0[..., None]
        bloom = np.maximum(bg - threshold, 0) * (1 - alpha0)
        for i in range(3):
            # bloom = np.maximum(bloom, gaussian_filter(bloom, 3, axes=[0, 1]))
            # bloom = np.maximum(bloom, nfilt(bloom, 2**i))
            bloom = nfilt(bloom, 2**i)
        blended = np.clip(bg * (1 - alpha0) + fg *
                          alpha0 + bloom * intensity, 0, 1)
    if False:
        treshold = 0.7
        alpha0 = alpha0[..., None]
        alpha2 = (1 - alpha0) * bg
        alpha1 = np.zeros_like(bg)
        for i in range(10):
            alpha2 = gaussian_filter(alpha2, i * 4, axes=[0, 1])
            # alpha2 = np.maximum(alpha2, nfilt(alpha2, 2**i))
            a = alpha1
            alpha1 = np.minimum(alpha1 + alpha2, treshold)
            alpha2 -= (alpha1 - a)
        # blended = bg2 * (1 - alpha1) + fg * alpha1
        blended = np.clip(alpha1 / treshold + fg * alpha0, 0, 1)
    if False:
        alpha0 = alpha0[..., None]
        alpha2 = (1 - alpha0) * bg
        alpha1 = np.zeros_like(bg)
        for i in range(5):
            p = alpha2
            alpha2 = gaussian_filter(alpha2, 3, axes=[0, 1])
            alpha2 = np.maximum(alpha2, p)
        alpha1 = alpha2
        blended = np.clip(alpha1 + fg * np.minimum(1 - alpha1, alpha0), 0, 1)
    if False:
        alpha0 = alpha0[..., None]
        blended = np.clip(bg * (1 - alpha0) + fg * alpha0, 0, 1)
        blended *= 1.2
        for i in range(10):
            alpha2 = np.maximum(0, blended - 1)
            alpha2 = gaussian_filter(alpha2, i * 2, axes=[0, 1])
            blended = np.clip(blended, 0, 1) + alpha2
        blended = np.clip(blended, 0, 1)
    plt.imshow(blended)
    plt.show()


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--background', required=True)
    parser.add_argument('--foreground', required=True)
    parser.add_argument('--scale', type=float, default=1.5)
    parser.add_argument('--offset', type=float, default=-0.5)
    parser.add_argument('--std', type=float, default=40)
    args = parser.parse_args()
    run(args)
