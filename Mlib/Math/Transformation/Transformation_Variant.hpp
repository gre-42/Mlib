#pragma once
#include <Mlib/Math/Transformation/Offset_And_YAngle.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <iosfwd>
#include <variant>

namespace Mlib {

template <class TDir, class TPos, size_t tndim>
class TransformationVariant {
public:
    std::variant<
        TransformationMatrix<TDir, TPos, tndim>,
        OffsetAndYAngle<TDir, TPos, 3>,
        TranslationMatrix<TPos, tndim>> data;
};

template <class TDir, class TPos, size_t tndim>
TransformationVariant<TDir, TPos, tndim> operator * (
    const TranslationMatrix<TPos, tndim>& p,
    const TransformationVariant<TDir, TPos, tndim>& m)
{
    return {std::visit([&p](const auto& d){ return TransformationVariant<TDir, TPos, tndim>{p * d}; }, m.data)};
}

template <class TDir, class TPos, size_t tndim>
TransformationVariant<TDir, TPos, tndim> operator * (
    const TransformationVariant<TDir, TPos, tndim>& m,
    const TranslationMatrix<TPos, tndim>& p)
{
    return {std::visit([&p](const auto& d){ return TransformationVariant<TDir, TPos, tndim>{d * p}; }, m.data)};
}

template <class TDir, class TPos, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const TransformationVariant<TDir, TPos, tndim>& v) {
    std::visit([&ostr](const auto& d){ ostr << d; }, v.data);
    return ostr;
}

}
