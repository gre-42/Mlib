#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

/**
 * Source: https://en.wikipedia.org/wiki/YUV
 */
Array<float> rgb2yuv(const Array<float>& rgb) {
    Array<float> m{
        {0.2126, 0.7152, 0.0722},
        {-0.09991, -0.33609, 0.436},
        {0.615, -0.55861,  -0.05639}};
    assert(rgb.shape(0) == 3);
    return batch_dot(m, rgb);
}

/**
 * Source: https://en.wikipedia.org/wiki/YUV
 */
Array<float> yuv2rgb(const Array<float>& yuv) {
    Array<float> m{
        {1, 0, 1.28033},
        {1, -0.21482, -0.38059},
        {1, 2.12798, 0}};
    assert(yuv.shape(0) == 3);
    return batch_dot(m, yuv);
}

}
