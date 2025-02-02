import imageio.v3 as iio
import matplotlib.pyplot as plt
import numpy as np
import re
from dataclasses import dataclass

# From: https://andrewwalker.github.io/statefultransitions/post/gaussian-fields
def fft_indgen(n):
    a = range(0, n//2+1)
    b = range(1, n//2)[::-1]
    b = [-i for i in b]
    return list(a) + b


def gaussian_random_field(Pk = lambda k : k**-3.0, size = (100, 100)):
    def Pk2(kx, ky):
        if kx == 0 and ky == 0:
            return 0.0
        return np.sqrt(Pk(np.sqrt(kx**2 + ky**2)))
    noise = np.fft.fft2(np.random.normal(size = size))
    amplitude = np.zeros(size)
    for i, kx in enumerate(fft_indgen(size[0])):
        for j, ky in enumerate(fft_indgen(size[1])):            
            amplitude[i, j] = Pk2(kx * size[1] / size[0], ky)
    return np.fft.ifft2(noise * amplitude)


def upsample1d(a: np.ndarray) -> np.ndarray:
    a = a.astype(float)
    res = np.empty(shape=(2 * a.shape[0] - 1,))
    res[1::2] = (a[:-1] + a[1:]) / 2.0
    res[::2] = a
    return res


def upsample(a: np.ndarray, axis: int) -> np.ndarray:
    return np.apply_along_axis(upsample1d, axis, a.astype(float))


def upsample2d(a: np.ndarray) -> np.ndarray:
    a = upsample(a, axis=0)
    a = upsample(a, axis=1)
    return a


def downsample1d(a: np.ndarray) -> np.ndarray:
    a = a.astype(float)
    res = np.empty(shape=((a.shape[0] + 1) // 2,))
    res[0] = (a[0] + a[1]) / 2.0
    res[-1] = (a[-2] + a[-1]) / 2.0
    res[1:] = (a[1:-2:2] + a[2:-1:2] + a[3::2]) / 3.0
    return res


def downsample(a: np.ndarray, axis: int) -> np.ndarray:
    return np.apply_along_axis(downsample1d, axis, a.astype(float))


def downsample2d(a: np.ndarray) -> np.ndarray:
    a = downsample(a, axis=0)
    a = downsample(a, axis=1)
    return a


@dataclass
class ResampledImage:
    filename: str
    downsample: int

    def __init__(self, f) -> None:
        m = re.match('^(.*)\[(-?\d+)\]$', f)
        if m is None:
            self.filename = f
            self.downsample = 0
        else:
            self.filename = m.group(1)
            self.downsample = int(m.group(2))
    
    def image(self, width, height) -> np.ndarray:
        res = iio.imread(self.filename)
        if self.downsample > 0:
            for i in range(self.downsample):
                res = downsample2d(res)
        if self.downsample < 0:
            for i in range(-self.downsample):
                res = upsample2d(res)
        if (width is not None) and (height is not None):
            R = (height - 1) // res.shape[0] + 1
            C = (width - 1) // res.shape[1] + 1
            res = np.tile(res, (R, C) + (1,) * (res.ndim - 2))[:height, :width, ...]
        return res


def ramp_2_alpha(
        ramp: np.ndarray,
        detail: np.ndarray,
        fac: float,
        offset: float) -> np.ndarray:
    blend = 1 - 2 * np.abs(ramp - 0.5)
    return np.clip(fac * (detail + offset) * blend + ramp, 0, 1)


def blend(
        alpha: np.ndarray,
        foreground: np.ndarray,
        background: np.ndarray) -> np.ndarray:
    return alpha[:, :, None] * foreground + (1 - alpha)[:, :, None] * background


def plot_2d(args):
    if args.foreground is None:
        foreground = 255 * np.ones((args.height, args.width, 1))
        background = np.zeros((args.height, args.width, 1))
    else:
        foreground = args.foreground.image(args.width, args.height)
        background = args.background.image(args.width, args.height)

    alpha = None
    for detail, fac, offset in zip(args.detail, args.detail_fac, args.detail_offset):
        if detail.filename == '<linear>':
            img = np.repeat(
                [np.linspace(0, 1, background.shape[1])],
                axis=0,
                repeats=background.shape[0])
        elif detail.filename == '<bilinear>':
            img = np.repeat(
                [np.concatenate([
                    np.linspace(0, 1, (background.shape[1] + 1) // 2),
                    np.linspace(1, 0, background.shape[1] // 2)])],
                axis=0,
                repeats=background.shape[0])
        elif detail.filename == '<grf>':
            img = np.real(gaussian_random_field(size=(background.shape[0], background.shape[1])))
        elif detail.filename == '<bigrf>':
            img = np.concatenate(
                [np.real(gaussian_random_field(size=(background.shape[0], (background.shape[1] + 1) // 2))),
                 np.real(gaussian_random_field(size=(background.shape[0], background.shape[1] // 2)))],
                axis=1)
        else:
            img = detail.image(args.width, args.height) / 255
            if img.ndim == 3:
                img = np.mean(img, axis=2)
            print(f'{detail}: Median={np.median(img)}')
        if alpha is None:
            alpha = np.clip(0.5 + fac * (img + offset), 0, 1)
        else:
            alpha = ramp_2_alpha(
                alpha,
                img,
                fac,
                offset)

    if args.flip_horizontally:
        details = detail[:, ::-1]

    #if False:
    #    foreground = np.tile(foreground[::2, ::2, :], reps=(4, 1, 1))
    #    background = np.tile(background[::2, ::2, :], reps=(4, 1, 1))
    #if True:
    #    foreground = np.tile(foreground[:, (foreground.shape[1]//2):, :], reps=(2, 1, 1))
    #    background = np.tile(background[:, (background.shape[1]//2):, :], reps=(2, 1, 1))
    #alpha = np.tile(alpha[:, (alpha.shape[1]//2):], reps=(2, 1))

    res = blend(alpha, foreground, background)
    if args.foreground is None:
        res = res[:, :, 0]
    if args.result is not None:
        iio.imwrite(args.result, res.clip(0, 255).astype(np.uint8))
    else:
        plt.imshow(res.clip(0, 255).astype(np.uint8))
        plt.show()


if __name__ == '__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser()
    # parser.add_argument('--ramp', required=True)
    # parser.add_argument('--blend', required=True)
    parser.add_argument('--detail', nargs='+', required=True, type=ResampledImage)
    parser.add_argument('--foreground', type=ResampledImage)
    parser.add_argument('--background', type=ResampledImage)
    parser.add_argument('--width', type=int)
    parser.add_argument('--height', type=int)
    parser.add_argument('--result')
    parser.add_argument('--detail_fac', nargs='+', type=float, required=True)
    parser.add_argument('--detail_offset', nargs='+', type=float, required=True)
    parser.add_argument('--flip_horizontally', action='store_true')

    args = parser.parse_args()

    plot_2d(args)
