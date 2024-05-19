#include "CvCompat.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Math/Math.hpp>
#include <algorithm>

using namespace Mlib;
using namespace ocv;

template <class TData>
Mat<TData>::Mat()
{}

template <class TData>
Mat<TData>::Mat(const Mlib::Array<TData>& array)
: array(array)
{}

template <class TData>
int Mat<TData>::channels() const {
    if (array.ndim() == 3) {
        return (int)array.shape(2);
    } else if (array.ndim() == 2) {
        return 1;
    } else {
        THROW_OR_ABORT("Unsupported image dimensions");
    }
}

template <class TData>
void Mat<TData>::copyTo(Mat<TData>& other) const {
    other.array = array;
}

template <class TData>
template <class TDestData>
void Mat<TData>::convertTo(Mat<TDestData>& dest, double alpha) const {
    if (alpha == 1) {
        dest.array.move() = array.template casted<TDestData>();
    } else {
        dest.array.move() = array.template applied<TDestData>([alpha](const TData& v){return (TDestData)(v * alpha);});
    }
}

template <class TData>
int Mat<TData>::rows() const {
    assert(array.ndim() == 2 || array.ndim() == 3);
    return (int)array.shape(0);
}

template <class TData>
int Mat<TData>::cols() const {
    assert(array.ndim() == 2 || array.ndim() == 3);
    return (int)array.shape(1);
}

template <class TData>
int Mat<TData>::step1() const {
    assert(array.ndim() == 2);
    return (int)array.shape(1);
}

template <class TData>
TData* Mat<TData>::ptr(int r) {
    assert(array.ndim() == 2);
    return &array((size_t)r, 0);
}

template <class TData>
const TData* Mat<TData>::ptr(int r) const {
    assert(array.ndim() == 2);
    return &array((size_t)r, 0);
}

template <class TData>
bool Mat<TData>::empty() const {
    return !array.initialized();
}

template <class TSourceData, class TDestData>
void Mlib::ocv::cvtColor(
    const Mat<TSourceData>& img,
    Mat<TDestData>& gray,
    ColorConversionCodes code)
{
    if (code == COLOR_BGR2GRAY) {
        assert_true(img.channels() == 3);
        gray.array.move() = sum(img.array, 2);
    } else {
        THROW_OR_ABORT("Unknown conversion code");
    }
}

template <class TData>
void Mlib::ocv::GaussianBlur(const Mat<TData>& src, Mat<TData>& dest, float sigma) {
    dest.array.move() = gaussian_filter_NWE(src.array, sigma, std::numeric_limits<TData>::max());
}

template <class TData>
void Mlib::ocv::exp(const TData* src, TData* dst, int len) {
    for (int i = 0; i < len; ++i) {
        dst[i] = std::exp(src[i]);
    }
}

template <class TData>
void Mlib::ocv::fastAtan2(const TData* Y, const TData* X, TData* Ori, int len) {
    for (int i = 0; i < len; ++i) {
        Ori[i] = std::atan2(Y[i], X[i]) * 180.f / float(M_PI);
        if (Ori[i] < 0.f) {
            Ori[i] += 360.f;
        }
        if (Ori[i] >= 360.f) {
            Ori[i] -= 360.f;
        }
    }
}

template <class TData>
void Mlib::ocv::magnitude(const TData* X, const TData* Y, TData* Mag, int len) {
    for (int i = 0; i < len; ++i) {
        Mag[i] = std::sqrt(squared(X[i]) + squared(Y[i]));
    }
}

template <class TData>
int Mlib::ocv::cvRound(const TData& data) {
    return (int)std::round(data);
}

int Mlib::ocv::cvFloor(float v) {
    return (int)std::floor(v);
}

template <class TDest, class TSource>
TDest Mlib::ocv::saturate_cast(const TSource& a) {
    return (TDest)std::clamp<TSource>(a, std::numeric_limits<TDest>::min(), std::numeric_limits<TDest>::max());
}

template uint8_t Mlib::ocv::saturate_cast<uint8_t, float>(const float& a);
template void Mlib::ocv::cvtColor<uint8_t, uint8_t>(const Mat<uint8_t>& img, Mat<uint8_t>& gray, ColorConversionCodes code);
template class Mlib::ocv::Mat<uint8_t>;
template class Mlib::ocv::Mat<int16_t>;
template class Mlib::ocv::Mat<float>;

template void Mlib::ocv::Mat<uint8_t>::convertTo<int16_t>(Mat<int16_t>& dest, double alpha) const;
template void Mlib::ocv::Mat<uint8_t>::convertTo<float>(Mat<float>& dest, double alpha) const;

template void Mlib::ocv::exp<float>(const float* src, float* dst, int len);
template void Mlib::ocv::fastAtan2<float>(const float* Y, const float* X, float* Ori, int len);
template void Mlib::ocv::magnitude<float>(const float* X, const float* Y, float* Mag, int len);
template int Mlib::ocv::cvRound(const float& data);
template int Mlib::ocv::cvRound(const double& data);

template void Mlib::ocv::GaussianBlur<int16_t>(const Mat<int16_t>& src, Mat<int16_t>& dest, float sigma);
template void Mlib::ocv::GaussianBlur<float>(const Mat<float>& src, Mat<float>& dest, float sigma);
