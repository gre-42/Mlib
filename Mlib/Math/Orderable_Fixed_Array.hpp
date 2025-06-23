#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Fixed_Array_Hash.hpp>
#include <Mlib/Default_Uninitialized.hpp>
#include <Mlib/Default_Uninitialized_Element.hpp>
#include <Mlib/Hash.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Std_Hash.hpp>
#include <compare>
#include <concepts>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;

template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray<TData, tshape0, tshape...>: public FixedArray<TData, tshape0, tshape...> {
public:
    using Base = FixedArray<TData, tshape0, tshape...>;
    using value_type = Base::value_type;
    using initializer_type = Base::initializer_type;

    OrderableFixedArray(NanInitialized) {
        *this = TData(nan_initialized);
    }
    OrderableFixedArray(Uninitialized)
        : Base{ uninitialized }
    {}
    template<std::convertible_to<TData> Value>
    explicit OrderableFixedArray(const Value& value)
        : Base{ value }
    {}
    template<std::convertible_to<initializer_type>... Values>
        requires (sizeof...(Values) == tshape0)
    OrderableFixedArray(const Values&... values)
        : Base{ values... }
    {}
    explicit OrderableFixedArray(const Base& rhs)
        : Base{ rhs }
    {}
    OrderableFixedArray& operator = (const TData& rhs) {
        Base& f = *this;
        f = rhs;
        return *this;
    }
    OrderableFixedArray& operator = (const Base& rhs) {
        Base& f = *this;
        f = rhs;
        return *this;
    }
    bool operator < (const OrderableFixedArray& rhs) const {
        return this->less_than(rhs);
    }
    bool operator == (const OrderableFixedArray& rhs) const {
        const Base& lhs = *this;
        const Base& rhs2 = rhs;
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
        const Base& a = *this;
        return all(a == d);
    }
};

template <class TData, size_t... tshape>
OrderableFixedArray<TData, tshape...> make_orderable(const FixedArray<TData, tshape...>& a) {
    return OrderableFixedArray<TData, tshape...>{ a };
}

template <class TData, size_t... tshape>
using UOrderableFixedArray = DefaultUnitialized<OrderableFixedArray<TData, tshape...>>;

template <class TData, size_t... tshape>
using EOrderableFixedArray = DefaultUnitializedElement<OrderableFixedArray<TData, tshape...>>;

}

template <class TData, size_t... tshape>
struct std::hash<Mlib::OrderableFixedArray<TData, tshape...>>
{
    std::size_t operator() (const Mlib::OrderableFixedArray<TData, tshape...>& a) const {
        return Mlib::fixed_array_hash(a);
    }
};
