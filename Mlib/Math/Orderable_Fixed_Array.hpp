#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Math.hpp>
#include <compare>

namespace Mlib {

template <class TData, size_t tlength>
class OrderableFixedArray: public FixedArray<TData, tlength> {
public:
    explicit OrderableFixedArray() {}
    explicit OrderableFixedArray(const FixedArray<TData, tlength>& rhs)
    : FixedArray<TData, tlength>{rhs}
    {}
    template<typename... Values>
    OrderableFixedArray(const TData& v0, const Values&... values)
    : FixedArray<TData, tlength>{v0, values...}
    {}
    OrderableFixedArray& operator = (const FixedArray<TData, tlength>& rhs) {
        FixedArray<TData, tlength>& f = *this;
        f = rhs;
        return *this;
    }
    bool operator < (const OrderableFixedArray& rhs) const {
        for(size_t i = 0; i < tlength; ++i) {
            if ((*this)(i) < rhs(i)) {
                return true;
            }
            if ((*this)(i) > rhs(i)) {
                return false;
            }
        }
        return false;
    }
    bool operator == (const OrderableFixedArray& rhs) const {
        const FixedArray<TData, tlength>& lhs = *this;
        const FixedArray<TData, tlength>& rhs2 = rhs;
        return all(lhs == rhs2);
    }
    bool operator != (const OrderableFixedArray& rhs) const {
        return !(*this == rhs);
    }
    bool operator > (const OrderableFixedArray& rhs) const {
        //return (*this != rhs) && (!(*this < rhs));
        return static_cast<const OrderableFixedArray&>(rhs) < *this;
    }
    std::strong_ordering operator <=> (const OrderableFixedArray& other) const {
        if (*this < other) {
            return std::strong_ordering::less;
        }
        if (*this == other) {
            return std::strong_ordering::equal;
        }
        return std::strong_ordering::greater;
    }
    bool is_nonzero() const {
        const FixedArray<TData, tlength>& a = *this;
        return any(a != TData{0});
    }
};

}
