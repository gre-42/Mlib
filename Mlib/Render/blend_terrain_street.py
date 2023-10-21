def plot_1d():
    import matplotlib.pyplot as plt
    import numpy as np

    x = np.linspace(-2, 2, 100)
    b = np.maximum(0, 1 - np.abs(x))
    d = np.square(np.sin(2 * x * 10))
    r = np.cumsum(b)
    r /= np.max(r)
    # plt.plot(x, r, x, b, x, np.clip((1 - b) * r + b * d, -1, 1))
    plt.plot(x, r, x, b, x, np.clip(r + b * d - b * r, -1, 1))
    plt.show()


def plot_2d(args):
    import imageio.v3 as iio
    import numpy as np

    # ramp = iio.imread(args.ramp)
    # blend = iio.imread(args.blend)
    # blend = gaussian_filter
    detail = args.detail_fac * iio.imread(args.detail) / 255 - args.detail_offset
    if args.flip_horizontally:
        details = detail[:, ::-1]
    foreground = iio.imread(args.foreground)
    background = iio.imread(args.background)

    x = np.repeat([np.linspace(-2, 2, detail.shape[1])],
                  axis=0, repeats=detail.shape[0])
    blend = np.maximum(0, 1 - np.abs(x))
    ramp = np.cumsum(blend, axis=1)
    ramp /= np.max(ramp)

    alpha = detail * blend + ramp
    res = alpha[:, :, None] * foreground + (1 - alpha)[:, :, None] * background
    iio.imwrite(args.result, res.clip(0, 255).astype(np.uint8))


if __name__ == '__main__':
    # --ramp data/textures/Way_Alpha3.png --blend ? data/textures/Way_Alpha3.png
    from argparse import ArgumentParser
    parser = ArgumentParser()
    # parser.add_argument('--ramp', required=True)
    # parser.add_argument('--blend', required=True)
    parser.add_argument('--detail', required=True)
    parser.add_argument('--foreground', required=True)
    parser.add_argument('--background', required=True)
    parser.add_argument('--result', required=True)
    parser.add_argument('--detail_fac', type=float, required=True)
    parser.add_argument('--detail_offset', type=float, required=True)
    parser.add_argument('--flip_horizontally', action='store_true')

    args = parser.parse_args()

    plot_2d(args)
