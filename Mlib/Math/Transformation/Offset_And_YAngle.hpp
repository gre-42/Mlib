#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>

namespace Mlib {

template <class TAngle, class TPos, size_t n>
class OffsetAndYAngle {
public:
    FixedArray<TPos, n> t;
    TAngle yangle;
    template <class TDestAngle, class TDestPos>
    OffsetAndYAngle<TDestAngle, TDestPos, n> casted() const {
        return {t.template casted<TDestPos>(), (TDestAngle)yangle};
    }
};

template <class TAngle, class TPos, size_t n>
OffsetAndYAngle<TAngle, TPos, n> operator * (const TranslationMatrix<TPos, n>& a, const OffsetAndYAngle<TAngle, TPos, n>& b) {
    return OffsetAndYAngle<TAngle, TPos, n>{a.t + b.t, b.yangle};
}

template <class TAngle, class TPos, size_t n>
std::ostream& operator << (std::ostream& ostr, const OffsetAndYAngle<TAngle, TPos, n>& oa) {
    ostr << oa.t << ", " << oa.yangle;
    return ostr;
}

}
