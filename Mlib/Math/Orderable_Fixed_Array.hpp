#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized.hpp>
#include <Mlib/Hash.hpp>
#include <Mlib/Math/Math.hpp>
#include <compare>
#include <concepts>

namespace Mlib {

template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray: public FixedArray<TData, tshape0, tshape...> {
public:
    OrderableFixedArray(Uninitialized)
        : FixedArray<TData, tshape0, tshape...>(uninitialized)
    {}
    explicit OrderableFixedArray(const FixedArray<TData, tshape0, tshape...>& rhs)
        : FixedArray<TData, tshape0, tshape...>{rhs}
    {}
    explicit OrderableFixedArray(const std::vector<TData>& rhs)
        : FixedArray<TData, tshape0, tshape...>{rhs}
    {}
    explicit OrderableFixedArray(const std::array<TData, FixedArray<TData, tshape0, tshape...>::nelements()>& rhs)
        : FixedArray<TData, tshape0, tshape...>{rhs}
    {}
    template<std::convertible_to<FixedArray<TData, tshape...>>... Values>
    OrderableFixedArray(const Values&... values)
        : FixedArray<TData, tshape0, tshape...>{values...}
    {}
    OrderableFixedArray& operator = (const TData& rhs) {
        FixedArray<TData, tshape0, tshape...>& f = *this;
        f = rhs;
        return *this;
    }
    OrderableFixedArray& operator = (const FixedArray<TData, tshape0, tshape...>& rhs) {
        FixedArray<TData, tshape0, tshape...>& f = *this;
        f = rhs;
        return *this;
    }
    bool operator < (const OrderableFixedArray& rhs) const {
        return this->less_than(rhs);
    }
    bool operator == (const OrderableFixedArray& rhs) const {
        const FixedArray<TData, tshape0, tshape...>& lhs = *this;
        const FixedArray<TData, tshape0, tshape...>& rhs2 = rhs;
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
    bool all_equal(const TData& d) const {
        const FixedArray<TData, tshape0, tshape...>& a = *this;
        return all(a == d);
    }
};

template <class TData, size_t... tshape>
using UOrderableFixedArray = DefaultUnitialized<OrderableFixedArray<TData, tshape...>>;

}

namespace std {

template <class Key>
struct hash;

}

template <class TData, size_t... tshape>
struct std::hash<Mlib::OrderableFixedArray<TData, tshape...>>
{
    std::size_t operator() (const Mlib::OrderableFixedArray<TData, tshape...>& a) const {
        Mlib::Hasher hasher{ 0xc0febabe };
        for (const auto& v : a.flat_iterable()) {
            hasher.combine(v);
        }
        return hasher;
    }
};
