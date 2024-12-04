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
struct IntermediateType<int16_t> {
    using type = float;
};

template <class T>
using intermediate_type = IntermediateType<T>::type;

static const struct Exclicit {} exclicit;

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
class ScaledInteger {
private:
    ScaledInteger(TInt count, Exclicit)
        : count{ count }
    {}
public:
    inline ScaledInteger() {}
    inline ScaledInteger(const ScaledInteger& other)
        : count{ other.count }
    {}
    static inline ScaledInteger from_count(TInt count) {
        return { count, exclicit };
    }
    inline ScaledInteger(const std::floating_point auto& value)
        : count{ static_cast<TInt>(std::round(value * denominator / numerator)) }
    {}
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
        return ScaledInteger<TInt2, numerator, denominator>::from_count((TInt2)count);
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
        return from_count(-count);
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
    return ScaledInteger<TInt, numerator, denominator>::from_count(a.count + b.count);
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
inline ScaledInteger<TInt, numerator, denominator> operator - (
        const ScaledInteger<TInt, numerator, denominator>& a,
        const ScaledInteger<TInt, numerator, denominator>& b)
{
    return ScaledInteger<TInt, numerator, denominator>::from_count(a.count - b.count);
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
    return ScaledInteger<TInt, numerator, denominator>::from_count((TInt)(a.count * b));
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator, std::floating_point F>
inline ScaledInteger<TInt, numerator, denominator> operator * (
    const ScaledInteger<TInt, numerator, denominator>& a,
    F b)
{
    return ScaledInteger<TInt, numerator, denominator>::from_count((TInt)(std::round(a.count * b)));
}

template <std::integral TInt, std::intmax_t numerator, std::intmax_t denominator>
inline ScaledInteger<TInt, numerator, denominator> operator / (
    const ScaledInteger<TInt, numerator, denominator>& a,
    TInt b)
{
    return ScaledInteger<TInt, numerator, denominator>::from_count(a.count / b);
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
        using T = Mlib::ScaledInteger<TInt, numerator, denominator>;
    public:
        static T lowest() { return T::from_count(std::numeric_limits<TInt>::lowest()); };
        static T max() { return T::from_count(std::numeric_limits<TInt>::max()); };
    };
}
