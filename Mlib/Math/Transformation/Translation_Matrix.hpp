#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib {

template <class T, size_t n>
class TranslationMatrix {
public:
    explicit TranslationMatrix(const FixedArray<T, n>& t)
    : t_(t)
    {}
    const FixedArray<T, n>& t() {
        return t_;
    }
    const FixedArray<T, n>& t() const {
        return t_;
    }
private:
    FixedArray<T, n> t_;
};

template <class TData, size_t n>
class UpperLeftArray: public BaseDenseFixedArray<UpperLeftArray<TData, n>, TData, n, n> {
public:
    UpperLeftArray(const FixedArray<TData, n+1, n+1>& a)
    : a_{a}
    {}
    constexpr static FixedArrayShape<n, n> shape() {
        return FixedArrayShape<n, n>();
    }
    const UpperLeftArray& rows_as_1D() const {
        return *this;
    }
    const TData& operator () (size_t r, size_t c) const {
        return a_(r, c);
    }
private:
    const FixedArray<TData, n+1, n+1>& a_;
};

template <class T, size_t n>
FixedArray<T, n+1, n+1> operator * (const FixedArray<T, n+1, n+1>& a, const TranslationMatrix<T, n>& b) {
    auto result = a;
    auto Rb = dot1d(UpperLeftArray<T, n>{a}, b.t());
    for (size_t i = 0; i < n; ++i) {
        result(i, n) += Rb(i);
    }
    return result;
}

template <class TDir, class TPos, size_t n>
TransformationMatrix<TDir, TPos, n> operator * (const TransformationMatrix<TDir, TPos, n>& a, const TranslationMatrix<TPos, n>& b) {
    return TransformationMatrix<TDir, TPos, n>{a.R(), a.transform(b.t())};
}

}
