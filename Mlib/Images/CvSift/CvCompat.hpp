#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib::ocv {

enum ColorConversionCodes {
    COLOR_BGR2GRAY = 6, 
};

template <class TData>
class Mat {
public:
    Mat();
    explicit Mat(const Mlib::Array<TData>& array);
    int channels() const;
    void copyTo(Mat& other) const;
    template <class TDestData>
    void convertTo(Mat<TDestData>& dest, double alpha=1) const;
    Mlib::Array<TData> array;
    int rows() const;
    int cols() const;
    int step1() const;
    TData* ptr(int r);
    const TData* ptr(int r) const;
    bool empty() const;
};

template <class TSourceData, class TDestData>
void cvtColor(const Mat<TSourceData>& source, Mat<TDestData>& dest, ColorConversionCodes code);

template <class TData>
void GaussianBlur(const Mat<TData>& src, Mat<TData>& dest, float sigma);

template <class TData>
void exp(const TData* src, TData* dst, int len);

template <class TData>
void fastAtan2(const TData* Y, const TData* X, TData* Ori, int len);

template <class TData>
void magnitude(const TData* X, const TData* Y, TData* Mag, int len);

template <class TData>
int cvRound(const TData& data);

int cvFloor(float v);

template <class TDest, class TSource>
TDest saturate_cast(const TSource& a);

}
