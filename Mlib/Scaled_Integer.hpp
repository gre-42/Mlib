#pragma once
#include <cmath>
#include <concepts>
#include <concepts>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <ratio>

namespace Mlib {

template <class TInt>
struct IntermediateType;

template <>
struct IntermediateType<int32_t> {
    using type = double;
};

template <>
struct IntermediateType<uint16_t> {
    using type = float;
};

template <class T>
using intermediate_type = IntermediateType<T>::type;

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
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
    template <class TInt2>
    inline explicit operator ScaledInteger<TInt2, numerator, denominator> () const {
        return { (TInt2)count };
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

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
inline ScaledInteger<TInt, numerator, denominator> operator + (
    const ScaledInteger<TInt, numerator, denominator>& a,
    const ScaledInteger<TInt, numerator, denominator>& b)
{
    return { a.count + b.count };
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
inline ScaledInteger<TInt, numerator, denominator> operator - (
        const ScaledInteger<TInt, numerator, denominator>& a,
        const ScaledInteger<TInt, numerator, denominator>& b)
{
    return { a.count - b.count };
}

// template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
// inline ScaledInteger<TInt, numerator, denominator> operator * (
//     const ScaledInteger<TInt, numerator, denominator>& a,
//     const ScaledInteger<TInt, numerator, denominator>& b)
// {
//     using I = intermediate_type<TInt>;
//     return { (I)a * (I)b };
// }

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator, std::integral I>
inline ScaledInteger<TInt, numerator, denominator> operator * (
    const ScaledInteger<TInt, numerator, denominator>& a,
    I b)
{
    return { (TInt)(a.count * b) };
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator, std::floating_point F>
inline ScaledInteger<TInt, numerator, denominator> operator * (
    const ScaledInteger<TInt, numerator, denominator>& a,
    F b)
{
    return { (TInt)(std::round(a.count * b)) };
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
inline ScaledInteger<TInt, numerator, denominator> operator / (
    const ScaledInteger<TInt, numerator, denominator>& a,
    TInt b)
{
    return { a.count / b };
}

// template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
// inline ScaledInteger<TInt, numerator, denominator> operator / (
//         const ScaledInteger<TInt, numerator, denominator>& a,
//         const ScaledInteger<TInt, numerator, denominator>& b)
// {
//     using I = intermediate_type<TInt>;
//     return { (I)a / (I)b };
// }

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator, std::integral I>
inline ScaledInteger<TInt, numerator, denominator>& operator /= (
    ScaledInteger<TInt, numerator, denominator>& a,
    I n)
{
    a.count /= n;
    return a;
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
inline ScaledInteger<TInt, numerator, denominator>& operator *= (
    ScaledInteger<TInt, numerator, denominator>& a,
    TInt n)
{
    a.count *= n;
    return a;
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
inline std::ostream& operator << (std::ostream& ostr, const ScaledInteger<TInt, numerator, denominator>& i) {
    i.print(ostr);
    return ostr;
}

}

namespace std {
    template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
    class numeric_limits<Mlib::ScaledInteger<TInt, numerator, denominator>> {
    public:
        static Mlib::ScaledInteger<TInt, numerator, denominator> lowest() { return { std::numeric_limits<TInt>::lowest() }; };
        static Mlib::ScaledInteger<TInt, numerator, denominator> max() { return { std::numeric_limits<TInt>::max() }; };
    };
}
