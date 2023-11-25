import imageio.v3 as iio
import matplotlib.pyplot as plt
import numpy as np

def plot_1d():
    x = np.linspace(-2, 2, 100)
    b = np.maximum(0, 1 - np.abs(x))
    d = np.square(np.sin(2 * x * 10))
    r = np.cumsum(b)
    r /= np.max(r)
    # plt.plot(x, r, x, b, x, np.clip((1 - b) * r + b * d, -1, 1))
    plt.plot(x, r, x, b, x, np.clip(r + b * d - b * r, -1, 1))
    plt.show()


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
    foreground = iio.imread(args.foreground)
    background = iio.imread(args.background)

    alpha = None
    for detail, fac, offset in zip(args.detail, args.detail_fac, args.detail_offset):
        if detail == '<linear>':
            img = np.repeat(
                [np.linspace(0, 1, background.shape[1])],
                axis=0,
                repeats=background.shape[0])
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
    if args.result is not None:
        iio.imwrite(args.result, res.clip(0, 255).astype(np.uint8))
    else:
        plt.imshow(res.clip(0, 255).astype(np.uint8))
        plt.show()


if __name__ == '__main__':
    # --ramp data/textures/Way_Alpha3.png --blend ? data/textures/Way_Alpha3.png
    from argparse import ArgumentParser
    parser = ArgumentParser()
    # parser.add_argument('--ramp', required=True)
    # parser.add_argument('--blend', required=True)
    parser.add_argument('--detail', nargs='+', required=True)
    parser.add_argument('--foreground', required=True)
    parser.add_argument('--background', required=True)
    parser.add_argument('--result')
    parser.add_argument('--detail_fac', nargs='+', type=float, required=True)
    parser.add_argument('--detail_offset', nargs='+', type=float, required=True)
    parser.add_argument('--flip_horizontally', action='store_true')

    args = parser.parse_args()

    plot_2d(args)
