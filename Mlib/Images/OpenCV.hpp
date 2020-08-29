#pragma once

#ifdef WITH_OPENCV

#include <Mlib/Array/Array.hpp>
#include <opencv2/core.hpp>

namespace Mlib {

template <class TData>
cv::Mat_<TData> array_to_cv_mat(const Array<TData>& a) {
    assert(a.ndim() == 2);
    auto res = cv::Mat_<TData>((int)a.shape(0), (int)a.shape(1));
    memcpy(&*res.begin(), a.flat_iterable().begin(), a.nbytes());
    return res;
}

template <class TData>
Array<TData> cv_mat_to_array(const cv::Mat_<TData>& m) {
    assert(m.size.dims() == 2);
    Array<TData> res{ArrayShape{(size_t)m.size[0], (size_t)m.size[1]}};
    memcpy(res.flat_iterable().begin(), &*m.begin(), res.nbytes());
    return res;
}

}

#endif
