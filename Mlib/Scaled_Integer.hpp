#pragma once
#include <cmath>
#include <concepts>
#include <iosfwd>
#include <iostream>
#include <ratio>

namespace Mlib {

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
class ScaledInteger {
public:
    inline ScaledInteger(TInt count)
        : count_{ count }
    {}
    template <std::floating_point TFloat>
    inline ScaledInteger(const TFloat& value)
        : count_{ static_cast<TInt>(std::round(value * denominator / numerator)) } {}
    template <std::floating_point TFloat>
    inline explicit operator TFloat () const {
        return TFloat(count_) * numerator / denominator;
    }
    inline ScaledInteger& operator += (const ScaledInteger& other) {
        count_ += other.count_;
        return *this;
    }
    inline ScaledInteger& operator -= (const ScaledInteger& other) {
        count_ -= other.count_;
        return *this;
    }
    inline TInt count() const {
        return count_;
    }
    inline bool operator < (const ScaledInteger& other) const {
        return count_ < other.count_;
    }
    inline bool operator > (const ScaledInteger& other) const {
        return count_ > other.count_;
    }
    inline bool operator == (const ScaledInteger& other) const {
        return count_ == other.count_;
    }
    inline void print(std::ostream& ostr) const {
        ostr << (double)(*this);
    }
private:
    TInt count_;
};

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> operator + (
    const ScaledInteger<TInt, numerator, denominator>& a,
    const ScaledInteger<TInt, numerator, denominator>& b)
{
    return { a.count() + b.count() };
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> operator - (
        const ScaledInteger<TInt, numerator, denominator>& a,
        const ScaledInteger<TInt, numerator, denominator>& b)
{
    return { a.count() - b.count() };
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
std::ostream& operator << (std::ostream& ostr, const ScaledInteger<TInt, numerator, denominator>& i) {
    i.print(ostr);
    return ostr;
}

}
