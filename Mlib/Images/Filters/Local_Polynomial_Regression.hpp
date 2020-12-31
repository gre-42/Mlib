#pragma once
#include <Mlib/Math/Batch_Math.hpp>
#include <set>

namespace Mlib {

template <class TData>
struct PolyDescriptor {
    std::list<size_t> indices;
    Array<TData> data;
};

template <class TData>
Array<Array<TData>> vanderNd(const Array<Array<TData>>& x, size_t degree) {
    assert(x.ndim() == 1);
    // degree, combination
    std::vector<std::vector<PolyDescriptor<TData>>> v(degree + 1);
    v[0].push_back(PolyDescriptor<TData>{
        .indices = {},
        .data = ones<TData>(x(0).shape())});
    size_t nelems = 1;
    for (size_t d = 1; d <= degree; ++d) {
        v[d].reserve(x.shape(0) * v[d-1].size());
        std::set<std::list<size_t>> inserted_indices;
        for (const auto& vv : v[d-1]) {
            for (size_t xid = 0; xid < x.length(); ++xid) {
                std::list<size_t> new_indices = vv.indices;
                new_indices.push_back(xid);
                new_indices.sort();
                if (!inserted_indices.contains(new_indices)) {
                    inserted_indices.insert(new_indices);
                    v[d].push_back(PolyDescriptor<TData>{
                        .indices = new_indices,
                        .data = x(xid) * vv.data});
                    ++nelems;
                }
            }
        }
    }
    size_t i = 0;
    Array<Array<TData>> result{ArrayShape{nelems}};
    for (size_t d = 0; d <= degree; ++d) {
        for (const auto& vv : v[d]) {
            result(i) = vv.data;
            ++i;
        }
    }
    return result;
}

template <class TImage>
void meshgrid(TImage& image, size_t axis) {
    if (image.ndim() == 2) {
        if (axis > 1) {
            throw std::runtime_error("Axis out of bounds");
        }
        size_t i[2];
        for (i[0] = 0; i[0] < image.shape(0); ++i[0]) {
            for (i[1] = 0; i[1] < image.shape(1); ++i[1]) {
                image(i[0], i[1]) = i[axis];
            }
        }
    } else {
        throw std::runtime_error("Unsupported meshgrid");
    }
}

template <class TData, class TFilter>
Array<TData> local_polynomial_regression(const Array<TData>& image, const TFilter& filter, size_t degree) {
    Array<Array<TData>> x{ArrayShape{image.ndim()}};
    for (size_t axis = 0; axis < x.shape(0); ++axis) {
        x(axis).resize(image.shape());
        Array<TData> tmp = x(axis);
        meshgrid(tmp, axis);
    }
    Array<Array<TData>> m = vanderNd(x, degree);
    Array<Array<TData>> xx(ArrayShape{m.shape(0), m.shape(0)});
    for (size_t r = 0; r < m.shape(0); ++r) {
        for (size_t c = 0; c < m.shape(0); ++c) {
            if (r >= c) {
                xx(r, c) = filter(m(r) * m(c));
            } else {
                xx(r, c) = xx(c, r);
            }
        }
    }
    Array<Array<TData>> b(ArrayShape{m.length()});
    for (size_t i = 0; i < b.length(); ++i) {
        b(i) = filter(m(i) * image);
    }
    Array<Array<TData>> beta = batch_solve_symm_1d(xx, b);
    return sum(m * beta);
}

}
