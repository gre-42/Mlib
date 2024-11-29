#pragma once
#include <cmath>
#include <concepts>
#include <iosfwd>
#include <iostream>
#include <ratio>

namespace Mlib {

template <class TInt>
struct IntermediateType;

template <>
struct IntermediateType<int32_t> {
    using type = double;
};

template <class T>
using intermediate_type = IntermediateType<T>::type;


template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
class ScaledInteger {
public:
    using float_type = intermediate_type<TInt>;
    inline ScaledInteger() {}
    inline ScaledInteger(const ScaledInteger& other)
        : count{ other.count }
    {}
    inline ScaledInteger(TInt count)
        : count{ count }
    {}
    inline ScaledInteger(const float_type& value) {
        *this = value;
    }
    inline ScaledInteger& operator = (const TInt& other) {
        count = other;
        return *this;
    }
    inline ScaledInteger& operator = (const float_type& value) {
        count = static_cast<TInt>(std::round(value * denominator / numerator));
        return *this;
    }
    inline ScaledInteger& operator = (const ScaledInteger& other) {
        count = other.count;
        return *this;
    }
    template <std::floating_point TFloat>
    inline explicit operator TFloat () const {
        return TFloat(count) * numerator / denominator;
    }
    inline ScaledInteger& operator += (const ScaledInteger& other) {
        count += other.count;
        return *this;
    }
    inline ScaledInteger& operator -= (const ScaledInteger& other) {
        count -= other.count;
        return *this;
    }
    inline ScaledInteger operator - () const {
        return { -count };
    }
    inline bool operator < (const ScaledInteger& other) const {
        return count < other.count;
    }
    inline bool operator > (const ScaledInteger& other) const {
        return count > other.count;
    }
    inline bool operator == (const ScaledInteger& other) const {
        return count == other.count;
    }
    inline bool operator <= (const ScaledInteger& other) const {
        return count <= other.count;
    }
    inline bool operator >= (const ScaledInteger& other) const {
        return count >= other.count;
    }
    inline bool operator != (const ScaledInteger& other) const {
        return count != other.count;
    }
    inline void print(std::ostream& ostr) const {
        ostr << (intermediate_type<TInt>)(*this);
    }
    TInt count;
};

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> operator + (
    const ScaledInteger<TInt, numerator, denominator>& a,
    const ScaledInteger<TInt, numerator, denominator>& b)
{
    return { a.count + b.count };
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> operator - (
        const ScaledInteger<TInt, numerator, denominator>& a,
        const ScaledInteger<TInt, numerator, denominator>& b)
{
    return { a.count - b.count };
}

// template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
// ScaledInteger<TInt, numerator, denominator> operator * (
//     const ScaledInteger<TInt, numerator, denominator>& a,
//     const ScaledInteger<TInt, numerator, denominator>& b)
// {
//     using I = intermediate_type<TInt>;
//     return { (I)a * (I)b };
// }

template <class TInt, std::intmax_t numerator, std::intmax_t denominator, std::integral I>
ScaledInteger<TInt, numerator, denominator> operator * (
    const ScaledInteger<TInt, numerator, denominator>& a,
    I b)
{
    return { (TInt)(a.count * b) };
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator, std::floating_point F>
ScaledInteger<TInt, numerator, denominator> operator * (
    const ScaledInteger<TInt, numerator, denominator>& a,
    F b)
{
    return { (TInt)(std::round(a.count * b)) };
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> operator / (
    const ScaledInteger<TInt, numerator, denominator>& a,
    TInt b)
{
    return { a.count / b };
}

// template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
// ScaledInteger<TInt, numerator, denominator> operator / (
//         const ScaledInteger<TInt, numerator, denominator>& a,
//         const ScaledInteger<TInt, numerator, denominator>& b)
// {
//     using I = intermediate_type<TInt>;
//     return { (I)a / (I)b };
// }

template <class TInt, std::intmax_t numerator, std::intmax_t denominator, std::integral I>
ScaledInteger<TInt, numerator, denominator>& operator /= (
    ScaledInteger<TInt, numerator, denominator>& a,
    I n)
{
    a.count /= n;
    return a;
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator>& operator *= (
    ScaledInteger<TInt, numerator, denominator>& a,
    TInt n)
{
    a.count *= n;
    return a;
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
std::ostream& operator << (std::ostream& ostr, const ScaledInteger<TInt, numerator, denominator>& i) {
    i.print(ostr);
    return ostr;
}

}
