import imageio.v3 as iio
import matplotlib.pyplot as plt
import numpy as np

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
        foreground = iio.imread(args.foreground)
        background = iio.imread(args.background)

    alpha = None
    for detail, fac, offset in zip(args.detail, args.detail_fac, args.detail_offset):
        if detail == '<linear>':
            img = np.repeat(
                [np.linspace(0, 1, background.shape[1])],
                axis=0,
                repeats=background.shape[0])
        elif detail == '<bilinear>':
            img = np.repeat(
                [np.concatenate([
                    np.linspace(0, 1, (background.shape[1] + 1) // 2),
                    np.linspace(1, 0, background.shape[1] // 2)])],
                axis=0,
                repeats=background.shape[0])
        elif detail == '<grf>':
            img = np.real(gaussian_random_field(size=(background.shape[0], background.shape[1])))
        elif detail == '<bigrf>':
            img = np.concatenate(
                [np.real(gaussian_random_field(size=(background.shape[0], (background.shape[1] + 1) // 2))),
                 np.real(gaussian_random_field(size=(background.shape[0], background.shape[1] // 2)))],
                axis=1)
        else:
            img = iio.imread(detail) / 255
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
    parser.add_argument('--detail', nargs='+', required=True)
    parser.add_argument('--foreground')
    parser.add_argument('--background')
    parser.add_argument('--width', type=int)
    parser.add_argument('--height', type=int)
    parser.add_argument('--result')
    parser.add_argument('--detail_fac', nargs='+', type=float, required=True)
    parser.add_argument('--detail_offset', nargs='+', type=float, required=True)
    parser.add_argument('--flip_horizontally', action='store_true')

    args = parser.parse_args()

    plot_2d(args)
