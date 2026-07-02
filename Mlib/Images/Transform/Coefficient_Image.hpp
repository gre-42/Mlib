#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <algorithm>
#include <cstddef>

namespace Mlib {

template <class T>
struct Coeff {
    size_t r;
    size_t c;
    T coeff;
    inline bool operator > (const Coeff& rhs) const {
        return coeff > rhs.coeff;
    }
};

template <class T, size_t nmax>
struct Coeffs {
    size_t n;
    FixedArray<Coeff<T>, nmax> coeffs;
    Coeffs()
        : n{0}
        , coeffs{Coeff<T>{0, 0, NAN}}
    {}
    void add(const Coeff<T>& coeff) {
        if (n < nmax) {
            coeffs(n++) = coeff;
            std::push_heap(coeffs.flat_begin(), coeffs.flat_begin() + n, std::greater<Coeff<T>>());
        } else if (coeff > coeffs(0)) {
            std::pop_heap(coeffs.flat_begin(), coeffs.flat_end(), std::greater<Coeff<T>>());
            coeffs(nmax - 1) = coeff;
            std::push_heap(coeffs.flat_begin(), coeffs.flat_end(), std::greater<Coeff<T>>());
        }
    }
};

template <class T, size_t ncoeffs>
class CoefficientImage {
public:
    CoefficientImage(
        const FixedArray<size_t, 2>& canvas_shape,
        const FixedArray<size_t, 2>& fragment_shape)
        : canvas_{ArrayShape{canvas_shape(0), canvas_shape(1)}}
        , fragment_shape_{fragment_shape}
    {}
    void add(size_t r, size_t c, const Coeff<T>& coeff) {
        if ((r >= canvas_.shape(0)) || (c >= canvas_.shape(1))) {
            throw std::runtime_error((std::stringstream() <<
                "Destination coordinates out of bounds. r: " <<
                    r << ", c: " << c <<
                    ", dim: " << canvas_.shape()).str());
        }
        if ((coeff.r >= fragment_shape_(0)) || (coeff.c >= fragment_shape_(1))) {
            throw std::runtime_error("Template coordinates out of bounds");
        }
        canvas_(r, c).add(coeff);
    }
    Array<T> assemble(const Array<T>& fragment) {
        if (fragment.ndim() != 3) {
            throw std::runtime_error("Template image does not have 3 dimensions");
        }
        if (any(FixedArray<size_t, 2>{fragment.shape(1), fragment.shape(2)} != fragment_shape_)) {
            throw std::runtime_error("Template shape mismatch");
        }
        auto result = zeros<T>(ArrayShape{fragment.shape(0)}.concatenated(canvas_.shape()));
        for (size_t r = 0; r < canvas_.shape(0); ++r) {
            for (size_t c = 0; c < canvas_.shape(1); ++c) {
                T alpha = 0;
                auto& ce = canvas_(r, c);
                for (size_t i = 0; i < ce.n; ++i) {
                    const auto& f = ce.coeffs(i);
                    alpha += f.coeff;
                    for (size_t h = 0; h < fragment.shape(0); ++h) {
                        result(h, r, c) += fragment(h, f.r, f.c) * f.coeff;
                    }
                }
                for (size_t h = 0; h < fragment.shape(0); ++h) {
                    result(h, r, c) /= std::max(alpha, (T)1e-6);
                }
            }
        }
        return result;
    }
private:
    Array<Coeffs<T, ncoeffs>> canvas_;
    FixedArray<size_t, 2> fragment_shape_;
};

}
