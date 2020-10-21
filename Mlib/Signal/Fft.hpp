#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

/**
 * From: https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
 */
template <class TFloat>
void fft1d_inplace(Array<std::complex<TFloat>>& x) {
    assert(x.ndim() == 1);
    // DFT
    size_t N = x.length();
    size_t k = N;
    size_t n;
    TFloat thetaT = M_PI / N;
    std::complex<TFloat> phiT(std::cos(thetaT), -std::sin(thetaT));
    while (k > 1)
    {
        n = k;
        k >>= 1;
        phiT = phiT * phiT;
        std::complex<TFloat> T = 1;
        for (size_t l = 0; l < k; l++)
        {
            for (size_t a = l; a < N; a += n)
            {
                size_t b = a + k;
                std::complex<TFloat> t = x(a) - x(b);
                x(a) += x(b);
                x(b) = t * T;
            }
            T *= phiT;
        }
    }
    // Decimate
    size_t m = (size_t)log2(N);
    for (size_t a = 0; a < N; a++)
    {
        size_t b = a;
        // Reverse bits
        b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
        b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
        b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
        b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
        b = ((b >> 16) | (b << 16)) >> (32 - m);
        if (b > a)
        {
            std::complex<TFloat> t = x(a);
            x(a) = x(b);
            x(b) = t;
        }
    }
}

template <class TFloat>
void fft_inplace(Array<std::complex<TFloat>>& x) {
    for(size_t axis = 0; axis < x.ndim(); ++axis) {
        Array<std::complex<TFloat>> tmp{ArrayShape{x.shape(axis)}};
        x = x.apply_over_axis(axis, ApplyOverAxisType::NOREDUCE,
            [&x, &tmp, axis](size_t i, size_t k, const Array<std::complex<TFloat>>& xf, Array<std::complex<TFloat>>& rf)
            {
                for(size_t h = 0; h < x.shape(axis); ++h) {
                    tmp(h) = xf(i, h, k);
                }
                fft1d_inplace(tmp);
                for(size_t h = 0; h < x.shape(axis); ++h) {
                    rf(i, h, k) = tmp(h);
                }
            });
    }
}

// inverse fft (in-place)
template <class TFloat>
void ifft_inplace(Array<std::complex<TFloat>>& x)
{
    // conjugate the complex numbers
    x = x.applied([](const std::complex<TFloat>& v){return std::conj(v);});
 
    // forward fft
    fft_inplace(x);
 
    // conjugate the complex numbers again
    x = x.applied([](const std::complex<TFloat>& v){return std::conj(v);});
 
    // scale the numbers
    x /= x.nelements();
}

template <class TFloat>
Array<std::complex<TFloat>> fft(const Array<std::complex<TFloat>>& x) {
    Array<std::complex<TFloat>> result = x.copy();
    fft_inplace(result);
    return result;
}

template <class TFloat>
Array<std::complex<TFloat>> ifft(const Array<std::complex<TFloat>>& x) {
    Array<std::complex<TFloat>> result = x.copy();
    ifft_inplace(result);
    return result;
}

}
