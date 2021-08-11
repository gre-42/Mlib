#pragma once
#include <Mlib/Images/Mesh_Coordinates/Meshgrid.hpp>
#include <Mlib/Math/Batch_Math.hpp>

namespace Mlib {

template <class TData, class TFilter>
Array<TData> local_polynomial_regression(const Array<TData>& image, const TFilter& filter, size_t degree, const Array<TData>* weights = nullptr) {
    Array<Array<TData>> x(ArrayShape{image.ndim()});
    for (size_t axis = 0; axis < x.shape(0); ++axis) {
        x(axis).resize(image.shape());
        Array<TData> tmp = x(axis);
        meshgrid(tmp, axis);
    }
    Array<Array<TData>> v = vanderNd(x, degree);
    Array<Array<TData>> m = (weights == nullptr)
        ? v
        : v * (*weights);
    Array<Array<TData>> xx(ArrayShape{m.shape(0), m.shape(0)});
    for (size_t r = 0; r < m.shape(0); ++r) {
        for (size_t c = 0; c < m.shape(0); ++c) {
            if (r <= c) {
                xx(r, c) = filter(m(r) * m(c));
            } else {
                xx(r, c) = xx(c, r);
            }
        }
    }
    Array<Array<TData>> b(ArrayShape{m.length()});
    Array<TData> rhs = (weights == nullptr)
        ? image
        : image * (*weights);
    for (size_t i = 0; i < b.length(); ++i) {
        b(i) = filter(m(i) * rhs);
    }
    Array<Array<TData>> beta = batch_solve_symm_1d(xx, b);
    return sum(v * beta);
}

}
