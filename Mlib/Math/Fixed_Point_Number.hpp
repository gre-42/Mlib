#pragma once
#include <Mlib/Math/Round.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <ratio>
#include <sstream>
#include <string>

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

template <>
struct IntermediateTypes<int8_t> {
    using count_type = int16_t;
    using float_type = float;
};

template <class T>
using intermediate_count = IntermediateTypes<T>::count_type;

template <class T>
using intermediate_float = IntermediateTypes<T>::float_type;

static const struct Explicit {} explicit_;

template <std::integral TInt, std::intmax_t r_shift>
class FixedPointNumber {
private:
    constexpr static const std::intmax_t numerator = (r_shift < 0) ? (1 << -r_shift) : 1;
    constexpr static const std::intmax_t denominator = (r_shift > 0) ? (1 << r_shift) : 1;
    constexpr FixedPointNumber(TInt count, Explicit)
        : count{ count }
    {}
public:
    template <std::integral TInt2>
    using ReplacedInt = FixedPointNumber<TInt2, r_shift>;

    inline FixedPointNumber() {}
    inline constexpr FixedPointNumber(const FixedPointNumber& other)
        : count{ other.count }
    {}
    static constexpr inline FixedPointNumber from_count(TInt count) {
        return { count, explicit_ };
    }
    template <std::floating_point F>
    static constexpr inline FixedPointNumber from_float_safe(const F& value) {
        if (!std::isfinite(value)) {
            throw std::runtime_error("Floating-point value is not finite");
        }
        auto res = FixedPointNumber{ value };
        if (std::abs((F)res - value) > ((F)5 * numerator) / denominator) {
            throw std::runtime_error("Large deviation after converting to fixed point");
        }
        return res;
    }
    inline constexpr explicit FixedPointNumber(const std::floating_point auto& value)
        : count{ static_cast<TInt>(Mlib::round((value * denominator) / numerator)) }
    {}
    constexpr inline FixedPointNumber& operator = (const FixedPointNumber& other) {
        count = other.count;
        return *this;
    }
    template <std::floating_point TFloat>
    constexpr inline explicit operator TFloat () const {
        return (TFloat(count) * numerator) / denominator;
    }
    template <std::integral TInt2>
    constexpr inline explicit operator TInt2 () const {
        return (TInt2)(intermediate_float<TInt>)(*this);
    }
    template <class TInt2, std::intmax_t r_shift2>
    constexpr inline explicit operator FixedPointNumber<TInt2, r_shift2> () const {
        if constexpr (r_shift2 > r_shift) {
            if constexpr (std::numeric_limits<TInt2>::max() > std::numeric_limits<TInt>::max()) {
                return FixedPointNumber<TInt2, r_shift2>::from_count(TInt2(count) << (r_shift2 - r_shift));
            } else {
                return FixedPointNumber<TInt2, r_shift2>::from_count(TInt2(count << (r_shift2 - r_shift)));
            }
        } else {
            return FixedPointNumber<TInt2, r_shift2>::from_count(TInt2(count >> (r_shift - r_shift2)));
        }
    }
    constexpr inline FixedPointNumber& operator += (const FixedPointNumber& other) {
        count += other.count;
        return *this;
    }
    constexpr inline FixedPointNumber& operator -= (const FixedPointNumber& other) {
        count -= other.count;
        return *this;
    }
    constexpr inline FixedPointNumber operator - () const {
        return from_count(-count);
    }
    constexpr inline FixedPointNumber operator + () const {
        return from_count(+count);
    }
    constexpr inline bool operator < (const FixedPointNumber& other) const {
        return count < other.count;
    }
    constexpr inline bool operator > (const FixedPointNumber& other) const {
        return count > other.count;
    }
    constexpr inline bool operator == (const FixedPointNumber& other) const {
        return count == other.count;
    }
    constexpr inline bool operator <= (const FixedPointNumber& other) const {
        return count <= other.count;
    }
    constexpr inline bool operator >= (const FixedPointNumber& other) const {
        return count >= other.count;
    }
    constexpr inline bool operator != (const FixedPointNumber& other) const {
        return count != other.count;
    }
    constexpr inline void print(std::ostream& ostr) const {
        ostr << (intermediate_float<TInt>)(*this);
    }
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(count);
    }
    TInt count;
};

template <std::integral TInt, std::intmax_t r_shift>
inline FixedPointNumber<TInt, r_shift> operator + (
    const FixedPointNumber<TInt, r_shift>& a,
    const FixedPointNumber<TInt, r_shift>& b)
{
    return FixedPointNumber<TInt, r_shift>::from_count(a.count + b.count);
}

template <std::integral TInt, std::intmax_t r_shift>
inline intermediate_float<TInt> operator + (
    const intermediate_float<TInt>& a,
    const FixedPointNumber<TInt, r_shift>& b)
{
    return a + (intermediate_float<TInt>)b;
}

template <std::integral TInt, std::intmax_t r_shift>
inline FixedPointNumber<TInt, r_shift> operator - (
        const FixedPointNumber<TInt, r_shift>& a,
        const FixedPointNumber<TInt, r_shift>& b)
{
    return FixedPointNumber<TInt, r_shift>::from_count(a.count - b.count);
}

// template <class TInt, std::intmax_t r_shift>
// inline FixedPointNumber<TInt, r_shift> operator * (
//     const FixedPointNumber<TInt, r_shift>& a,
//     const FixedPointNumber<TInt, r_shift>& b)
// {
//     using I = intermediate_float<TInt>;
//     return { (I)a * (I)b };
// }

template <std::integral TInt, std::intmax_t r_shift, std::integral I>
inline FixedPointNumber<TInt, r_shift> operator * (
    const FixedPointNumber<TInt, r_shift>& a,
    I b)
{
    return FixedPointNumber<TInt, r_shift>::from_count((TInt)(a.count * b));
}

template <std::integral TInt, std::intmax_t r_shift, std::floating_point F>
inline FixedPointNumber<TInt, r_shift> operator * (
    const FixedPointNumber<TInt, r_shift>& a,
    F b)
{
    return (FixedPointNumber<TInt, r_shift>)((F)a * b);
}

template <std::integral TInt, std::intmax_t r_shift, std::floating_point F>
inline FixedPointNumber<TInt, r_shift> operator * (
    F a,
    const FixedPointNumber<TInt, r_shift>& b)
{
    return (FixedPointNumber<TInt, r_shift>)(a * (F)b);
}

// template <std::integral TInt, std::intmax_t r_shift>
// inline FixedPointNumber<intermediate_count<TInt>, r_shift> operator * (
//     const FixedPointNumber<TInt, r_shift>& a,
//     const FixedPointNumber<TInt, r_shift>& b)
// {
//     using I = intermediate_count<TInt>;
//     return FixedPointNumber<I, r_shift>::from_count(((I)a * (I)b) / r_shift);
// }

template <std::integral TInt, std::intmax_t r_shift, std::integral I>
inline FixedPointNumber<TInt, r_shift> operator / (
    const FixedPointNumber<TInt, r_shift>& a,
    I b)
{
    return FixedPointNumber<TInt, r_shift>::from_count(a.count / b);
}

// template <std::integral TInt, std::intmax_t r_shift>
// inline FixedPointNumber<TInt, r_shift> operator / (
//         const FixedPointNumber<TInt, r_shift>& a,
//         const FixedPointNumber<TInt, r_shift>& b)
// {
//     using I = intermediate_float<TInt>;
//     return { (I)a / (I)b };
// }

template <std::integral TInt, std::intmax_t r_shift, std::integral I>
inline FixedPointNumber<TInt, r_shift>& operator /= (
    FixedPointNumber<TInt, r_shift>& a,
    I n)
{
    a.count /= n;
    return a;
}

template <std::integral TInt, std::intmax_t r_shift>
inline FixedPointNumber<TInt, r_shift>& operator *= (
    FixedPointNumber<TInt, r_shift>& a,
    TInt n)
{
    a.count *= n;
    return a;
}

template <std::integral TInt, std::intmax_t r_shift, std::floating_point F>
inline FixedPointNumber<TInt, r_shift>& operator *= (
    FixedPointNumber<TInt, r_shift>& a,
    F b)
{
    a = a * b;
    return a;
}

template <std::integral TInt, std::intmax_t r_shift>
inline std::ostream& operator << (std::ostream& ostr, const FixedPointNumber<TInt, r_shift>& i) {
    i.print(ostr);
    return ostr;
}

template <std::integral TInt, std::intmax_t r_shift>
inline std::iostream& operator >> (std::iostream& istr, FixedPointNumber<TInt, r_shift>& i) {
    intermediate_float<TInt> f;
    istr >> f;
    if (!istr.fail()) {
        i = FixedPointNumber<TInt, r_shift>{ f };
    }
    return istr;
}

}

namespace std {
    template <std::integral TInt, std::intmax_t r_shift>
    class numeric_limits<Mlib::FixedPointNumber<TInt, r_shift>> {
        using T = Mlib::FixedPointNumber<TInt, r_shift>;
    public:
        static constexpr T lowest() { return T::from_count(std::numeric_limits<TInt>::lowest()); };
        static constexpr T min() { return T::from_count(1); };
        static constexpr T max() { return T::from_count(std::numeric_limits<TInt>::max()); };
    };

    template <std::integral TInt, std::intmax_t r_shift>
    std::string to_string(const Mlib::FixedPointNumber<TInt, r_shift>& v) {
        return (std::stringstream() << v).str();
    }

#ifndef __clang__
    template <std::integral TInt, std::intmax_t r_shift>
    std::from_chars_result from_chars( const char* first, const char* last,
        Mlib::FixedPointNumber<TInt, r_shift>& value,
        std::chars_format fmt = std::chars_format::general)
    {
        Mlib::intermediate_float<TInt> fvalue;
        auto result = std::from_chars(first, last, fvalue, fmt);
        if (result.ec == std::errc()) {
            value = Mlib::FixedPointNumber<TInt, r_shift>{fvalue};
        }
        return result;
    }
#endif
}
