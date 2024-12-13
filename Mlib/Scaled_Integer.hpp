#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <concepts>
#include <concepts>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <ratio>
#include <sstream>

namespace Mlib {

template <class TInt>
struct IntermediateTypes;

template <>
struct IntermediateTypes<int32_t> {
    using count_type = int64_t;
    using float_type = double;
};

template <>
struct IntermediateTypes<int16_t> {
    using count_type = int32_t;
    using float_type = float;
};

template <class T>
using intermediate_count = IntermediateTypes<T>::count_type;

template <class T>
using intermediate_float = IntermediateTypes<T>::float_type;

static const struct Explicit {} explicit_;

template <std::integral TInt, std::intmax_t denominator>
class ScaledInteger {
private:
    constexpr ScaledInteger(TInt count, Explicit)
        : count{ count }
    {}
public:
    inline ScaledInteger() {}
    inline constexpr ScaledInteger(const ScaledInteger& other)
        : count{ other.count }
    {}
    static constexpr inline ScaledInteger from_count(TInt count) {
        return { count, explicit_ };
    }
    template <std::floating_point F>
    static constexpr inline ScaledInteger from_float_safe(const F& value) {
        if (!std::isfinite(value)) {
            THROW_OR_ABORT("Floating-point value is not finite");
        }
        auto res = ScaledInteger{ value };
        if (std::abs((F)res - value) > (F)5 / denominator) {
            THROW_OR_ABORT("Large deviation after converting to fixed point");
        }
        return res;
    }
    inline constexpr explicit ScaledInteger(const std::floating_point auto& value)
        : count{ static_cast<TInt>(std::round(value * denominator)) }
    {}
    inline ScaledInteger& operator = (const ScaledInteger& other) {
        count = other.count;
        return *this;
    }
    template <std::floating_point TFloat>
    inline explicit operator TFloat () const {
        return TFloat(count) / denominator;
    }
    template <std::integral TInt2>
    inline explicit operator TInt2 () const {
        return (TInt2)(intermediate_float<TInt>)(*this);
    }
    template <class TInt2>
    inline explicit operator ScaledInteger<TInt2, denominator> () const {
        return ScaledInteger<TInt2, denominator>::from_count((TInt2)count);
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
    inline ScaledInteger operator + () const {
        return from_count(+count);
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
        ostr << (intermediate_float<TInt>)(*this);
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(count);
    }
    TInt count;
};

template <std::integral TInt, std::intmax_t denominator>
inline ScaledInteger<TInt, denominator> operator + (
    const ScaledInteger<TInt, denominator>& a,
    const ScaledInteger<TInt, denominator>& b)
{
    return ScaledInteger<TInt, denominator>::from_count(a.count + b.count);
}

template <std::integral TInt, std::intmax_t denominator>
inline intermediate_float<TInt> operator + (
    const intermediate_float<TInt>& a,
    const ScaledInteger<TInt, denominator>& b)
{
    return a + (intermediate_float<TInt>)b;
}

template <std::integral TInt, std::intmax_t denominator>
inline ScaledInteger<TInt, denominator> operator - (
        const ScaledInteger<TInt, denominator>& a,
        const ScaledInteger<TInt, denominator>& b)
{
    return ScaledInteger<TInt, denominator>::from_count(a.count - b.count);
}

// template <class TInt, std::intmax_t denominator>
// inline ScaledInteger<TInt, denominator> operator * (
//     const ScaledInteger<TInt, denominator>& a,
//     const ScaledInteger<TInt, denominator>& b)
// {
//     using I = intermediate_float<TInt>;
//     return { (I)a * (I)b };
// }

template <std::integral TInt, std::intmax_t denominator, std::integral I>
inline ScaledInteger<TInt, denominator> operator * (
    const ScaledInteger<TInt, denominator>& a,
    I b)
{
    return ScaledInteger<TInt, denominator>::from_count((TInt)(a.count * b));
}

template <std::integral TInt, std::intmax_t denominator, std::floating_point F>
inline ScaledInteger<TInt, denominator> operator * (
    const ScaledInteger<TInt, denominator>& a,
    F b)
{
    return (ScaledInteger<TInt, denominator>)((F)a * b);
}

template <std::integral TInt, std::intmax_t denominator, std::floating_point F>
inline ScaledInteger<TInt, denominator> operator * (
    F a,
    const ScaledInteger<TInt, denominator>& b)
{
    return (ScaledInteger<TInt, denominator>)(a * (F)b);
}

// template <std::integral TInt, std::intmax_t denominator>
// inline ScaledInteger<intermediate_count<TInt>, denominator> operator * (
//     const ScaledInteger<TInt, denominator>& a,
//     const ScaledInteger<TInt, denominator>& b)
// {
//     using I = intermediate_count<TInt>;
//     return ScaledInteger<I, denominator>::from_count(((I)a * (I)b) / denominator);
// }

template <std::integral TInt, std::intmax_t denominator, std::integral I>
inline ScaledInteger<TInt, denominator> operator / (
    const ScaledInteger<TInt, denominator>& a,
    I b)
{
    return ScaledInteger<TInt, denominator>::from_count(a.count / b);
}

// template <std::integral TInt, std::intmax_t denominator>
// inline ScaledInteger<TInt, denominator> operator / (
//         const ScaledInteger<TInt, denominator>& a,
//         const ScaledInteger<TInt, denominator>& b)
// {
//     using I = intermediate_float<TInt>;
//     return { (I)a / (I)b };
// }

template <std::integral TInt, std::intmax_t denominator, std::integral I>
inline ScaledInteger<TInt, denominator>& operator /= (
    ScaledInteger<TInt, denominator>& a,
    I n)
{
    a.count /= n;
    return a;
}

template <std::integral TInt, std::intmax_t denominator>
inline ScaledInteger<TInt, denominator>& operator *= (
    ScaledInteger<TInt, denominator>& a,
    TInt n)
{
    a.count *= n;
    return a;
}

template <std::integral TInt, std::intmax_t denominator, std::floating_point F>
inline ScaledInteger<TInt, denominator>& operator *= (
    ScaledInteger<TInt, denominator>& a,
    F b)
{
    a = a * b;
    return a;
}

template <std::integral TInt, std::intmax_t denominator>
inline std::ostream& operator << (std::ostream& ostr, const ScaledInteger<TInt, denominator>& i) {
    i.print(ostr);
    return ostr;
}

template <std::integral TInt, std::intmax_t denominator>
inline std::iostream& operator >> (std::iostream& istr, ScaledInteger<TInt, denominator>& i) {
    intermediate_float<TInt> f;
    istr >> f;
    if (!istr.fail()) {
        i = ScaledInteger<TInt, denominator>{ f };
    }
    return istr;
}

}

namespace std {
    template <std::integral TInt, std::intmax_t denominator>
    class numeric_limits<Mlib::ScaledInteger<TInt, denominator>> {
        using T = Mlib::ScaledInteger<TInt, denominator>;
    public:
        static constexpr T lowest() { return T::from_count(std::numeric_limits<TInt>::lowest()); };
        static constexpr T max() { return T::from_count(std::numeric_limits<TInt>::max()); };
    };

    template <std::integral TInt, std::intmax_t denominator>
    std::string to_string(const Mlib::ScaledInteger<TInt, denominator>& v) {
        return (std::stringstream() << v).str();
    }
}
