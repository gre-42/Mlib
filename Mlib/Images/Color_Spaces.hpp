#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

/**
 * Source: https://en.wikipedia.org/wiki/YUV
 */
Array<float> rgb2yuv(const Array<float>& rgb) {
    Array<float> m{
        {0.2126f, 0.7152f, 0.0722f},
        {-0.09991f, -0.33609f, 0.436f},
        {0.615f, -0.55861f,  -0.05639f}};
    assert(rgb.shape(0) == 3);
    return batch_dot(m, rgb);
}

/**
 * Source: https://en.wikipedia.org/wiki/YUV
 */
Array<float> yuv2rgb(const Array<float>& yuv) {
    Array<float> m{
        {1.f, 0.f, 1.28033f},
        {1.f, -0.21482f, -0.38059f},
        {1.f, 2.12798f, 0.f}};
    assert(yuv.shape(0) == 3);
    return batch_dot(m, yuv);
}

}
