#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Math.hpp>
#include <compare>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray: public FixedArray<TData, tshape...> {
public:
    explicit OrderableFixedArray() {}
    explicit OrderableFixedArray(const FixedArray<TData, tshape...>& rhs)
    : FixedArray<TData, tshape...>{rhs}
    {}
    template<typename... Values>
    OrderableFixedArray(const TData& v0, const Values&... values)
    : FixedArray<TData, tshape...>{v0, values...}
    {}
    OrderableFixedArray& operator = (const FixedArray<TData, tshape...>& rhs) {
        FixedArray<TData, tshape...>& f = *this;
        f = rhs;
        return *this;
    }
    bool operator < (const OrderableFixedArray& rhs) const {
        return this->less_than(rhs);
    }
    bool operator == (const OrderableFixedArray& rhs) const {
        const FixedArray<TData, tshape...>& lhs = *this;
        const FixedArray<TData, tshape...>& rhs2 = rhs;
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
        const FixedArray<TData, tshape...>& a = *this;
        return any(a != TData{0});
    }
};

}
