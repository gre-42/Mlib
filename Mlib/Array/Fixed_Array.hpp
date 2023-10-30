#pragma once
#include <Mlib/Array/Base_Dense_Fixed_Array.hpp>
#include <Mlib/Array/Fixed_Array_Shape.hpp>
#include <Mlib/Io/Write_Number.hpp>
#include <Mlib/Math/Conju.hpp>
#include <Mlib/Template.hpp>
#include <array>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <type_traits>
#include <vector>

namespace Mlib {

template <class TData>
class Array;
class ArrayShape;

template <typename TData>
class PointerIterable {
public:
    PointerIterable(TData* begin, TData* end)
    :begin_(begin),
     end_(end)
    {}
    TData* begin() {
        return begin_;
    }
    TData* end() {
        return end_;
    }
private:
    TData* begin_;
    TData* end_;
};

template <typename TData, size_t... tshape>
class FixedArray;

template <typename TData, size_t tshape0, size_t... tshape>
class FixedArray<TData, tshape0, tshape...>: public BaseDenseFixedArray<FixedArray<TData, tshape0, tshape...>, TData, tshape0, tshape...>
{
public:
    typedef TData value_type;

    static constexpr size_t nelements() {
        return tshape0 * FixedArray<TData, tshape...>::nelements();
    }
    static constexpr size_t nbytes() {
        return nelements() * sizeof(TData);
    }
    static constexpr size_t length() {
        static_assert(ndim() == 1);
        return tshape0;
    }
    static constexpr size_t ndim() {
        return shape().ndim();
    }

    FixedArray() {}
    explicit FixedArray(const TData& rhs) {
        for (TData& v : flat_iterable()) {
            v = rhs;
        }
    }
    explicit FixedArray(const Array<TData>& a) {
        assert(all(a.shape() == ArrayShape{tshape0, tshape...}));
        std::copy(a.flat_begin(), a.flat_end(), flat_begin());
    }
    explicit FixedArray(const std::vector<TData>& v) {
        assert(v.size() == nelements());
        std::copy(v.begin(), v.end(), flat_begin());
    }
    explicit FixedArray(const ArrayShape& shape);
    explicit FixedArray(const std::array<TData, FixedArray<TData, tshape0, tshape...>::nelements()>& v) {
        std::copy(v.begin(), v.end(), flat_begin());
    }
    explicit FixedArray(const TData* data, size_t nelements)
    {
        assert(nelements == this->nelements());
        std::copy(data, data + nelements, flat_begin());
    }
    template<typename... Values>
    FixedArray(const FixedArray<TData, tshape...>& v0, const Values&... values)
    : data_{v0, values...}
    {}
    template<typename... Values>
    static FixedArray<TData, tshape0, tshape...> init(const TData& v0, const Values&... values) {
        static_assert(std::is_trivially_constructible_v<TData>);
        static_assert(1 + sizeof...(values) == nelements());
        FixedArray<TData, tshape0, tshape...> result;
        result.set_values<0>(v0, values...);
        return result;
    }
    FixedArray& operator = (const TData& rhs) {
        for (TData& v : flat_iterable()) {
            v = rhs;
        }
        return *this;
    }
    template <typename... Ids>
    const TData& operator() (size_t id0, Ids... ids) const {
        assert(id0 < tshape0);
        return data_[id0](ids...);
    }
    template <typename... Ids>
    TData& operator() (size_t id0, Ids... ids) {
        assert(id0 < tshape0);
        return data_[id0](ids...);
    }
    const FixedArray<TData, tshape...>& operator [] (size_t id) const {
        assert(id < tshape0);
        return data_[id];
    }
    FixedArray<TData, tshape...>& operator [] (size_t id) {
        assert(id < tshape0);
        return data_[id];
    }
    constexpr PointerIterable<const TData> flat_iterable() const {
        return PointerIterable<const TData>{flat_begin(), flat_end()};
    }
    constexpr PointerIterable<TData> flat_iterable() {
        return PointerIterable<TData>{flat_begin(), flat_end()};
    }
    constexpr TData* flat_begin() {
        if (tshape0 == 0) {
            return nullptr;
        } else {
            return data_[0].flat_begin();
        }
    }
    constexpr TData* flat_end() {
        if (tshape0 == 0) {
            return nullptr;
        } else {
            return data_[tshape0 - 1].flat_end();
        }
    }
    constexpr const TData* flat_begin() const {
        return const_cast<FixedArray*>(this)->flat_begin();
    }
    constexpr const TData* flat_end() const {
        return const_cast<FixedArray*>(this)->flat_end();
    }
    template <class TResultData=TData, class TOperation>
    FixedArray<TResultData, tshape0, tshape...> applied(const TOperation &operation) const {
        FixedArray<TResultData, tshape0, tshape...> r;
        const TData* s = flat_begin();
        TResultData* d = r.flat_begin();
        for (size_t i = 0; i < nelements(); ++i) {
            *d++ = operation(*s++);
        }
        return r;
    }
    template <class TDataResult=TData, class TDataB, class TBinop>
    FixedArray<TDataResult, tshape0, tshape...> array_array_binop(const FixedArray<TDataB, tshape0, tshape...>& b, const TBinop &binop) const {
        FixedArray<TDataResult, tshape0, tshape...> r;
        const TData* sa = flat_begin();
        const TData* sb = b.flat_begin();
        TDataResult* d = r.flat_begin();
        for (size_t i = 0; i < nelements(); ++i) {
            *d++ = binop(*sa++, *sb++);
        }
        return r;
    }
    bool less_than(const FixedArray& rhs) const {
        for (size_t i = 0; i < tshape0; ++i) {
            if ((*this)[i].less_than(rhs[i])) {
                return true;
            }
            if (rhs[i].less_than((*this)[i])) {
                return false;
            }
        }
        return false;
    }
    FixedArray& operator += (const FixedArray& rhs) {
        auto v0 = flat_begin();
        auto v1 = rhs.flat_begin();
        while(v0 != flat_end()) {
            *v0++ += *v1++;
        }
        return *this;
    }
    FixedArray& operator -= (const FixedArray& rhs) {
        auto v0 = flat_begin();
        auto v1 = rhs.flat_begin();
        while(v0 != flat_end()) {
            *v0++ -= *v1++;
        }
        return *this;
    }
    FixedArray& operator *= (const FixedArray& rhs) {
        auto a = flat_begin();
        auto b = rhs.flat_begin();
        while(a != flat_end()) {
            *a++ *= *b++;
        }
        return *this;
    }
    FixedArray& operator /= (const FixedArray& rhs) {
        auto a = flat_begin();
        auto b = rhs.flat_begin();
        while(a != flat_end()) {
            *a++ /= *b++;
        }
        return *this;
    }
    FixedArray& operator *= (const TData& rhs) {
        for (TData& v : flat_iterable()) {
            v *= rhs;
        }
        return *this;
    }
    FixedArray& operator /= (const TData& rhs) {
        for (TData& v : flat_iterable()) {
            v /= rhs;
        }
        return *this;
    }
    template <size_t tstart, size_t tend>
    FixedArray<TData, tend - tstart, tshape...>& row_range() {
        static_assert(tend >= tstart);
        static_assert(tend <= tshape0);
        return reinterpret_cast<FixedArray<TData, tend - tstart, tshape...>&>((*this)[tstart]);
    }
    template <size_t tstart, size_t tend>
    const FixedArray<TData, tend - tstart, tshape...>& row_range() const {
        auto& x = const_cast<FixedArray<TData, tshape0, tshape...>&>(*this);
        return x TEMPLATEV row_range<tstart, tend>();
    }
    FixedArray<TData, tshape0> column(size_t c) const {
        static_assert(ndim() == 2);
        FixedArray<TData, tshape0> result;
        for (size_t r = 0; r < tshape0; ++r) {
            result(r) = (*this)(r, c);
        }
        return result;
    }
    template <size_t... tnew_shape>
    constexpr FixedArray<TData, tnew_shape...>& reshaped() {
        typedef FixedArray<TData, tnew_shape...> Result;
        static_assert(Result::nelements() <= nelements());
        return *reinterpret_cast<Result*>(this);
    }
    template <size_t... tnew_shape>
    constexpr const FixedArray<TData, tnew_shape...>& reshaped() const {
        return const_cast<FixedArray*>(this)->reshaped<tnew_shape...>();
    }
    constexpr static FixedArrayShape<tshape0, tshape...> shape() {
        return FixedArrayShape<tshape0, tshape...>();
    }
    template <size_t N>
    constexpr static size_t static_shape() {
        return shape() TEMPLATEV get<N>();
    }
    template <size_t... tnew_shape>
    constexpr FixedArray<TData, tnew_shape...>& reshaped(const FixedArrayShape<tnew_shape...>&) {
        return reshaped<tnew_shape...>();
    }
    template <size_t... tnew_shape>
    constexpr const FixedArray<TData, tnew_shape...>& reshaped(const FixedArrayShape<tnew_shape...>&) const {
        return reshaped<tnew_shape...>();
    }
    constexpr auto& rows_as_1D() {
        return reshaped(shape().rows_as_1D());
    }
    constexpr auto& columns_as_1D() {
        return reshaped(shape().columns_as_1D());
    }
    constexpr const auto& rows_as_1D() const {
        return reshaped(shape().rows_as_1D());
    }
    constexpr const auto& columns_as_1D() const {
        return reshaped(shape().columns_as_1D());
    }
    constexpr auto& as_row_vector() {
        static_assert(ndim() == 1);
        return reshaped(FixedArrayShape<1, tshape0>());
    }
    constexpr auto& as_column_vector() {
        static_assert(ndim() == 1);
        return reshaped(FixedArrayShape<tshape0, 1>());
    }
    constexpr const auto& as_row_vector() const {
        static_assert(ndim() == 1);
        return reshaped(FixedArrayShape<1, tshape0>());
    }
    constexpr const auto& as_column_vector() const {
        static_assert(ndim() == 1);
        return reshaped(FixedArrayShape<tshape0, 1>());
    }
    constexpr auto& flattened() {
        return reshaped(FixedArrayShape<nelements()>());
    }
    constexpr const auto& flattened() const {
        return reshaped(FixedArrayShape<nelements()>());
    }
    Array<TData> to_array() const {
        Array<TData> result{ ArrayShape{tshape0, tshape...} };
        std::copy(flat_begin(), flat_end(), result.flat_begin());
        return result;
    }
    std::array<TData, nelements()> to_std_array() const {
        std::array<TData, nelements()> result;
        std::copy(flat_begin(), flat_end(), result.begin());
        return result;
    }
    ArrayShape to_array_shape() const;
    ArrayShape array_shape() const;
    auto T() const {
        static_assert(ndim() == 2);
        FixedArray<TData, shape().last(), tshape0> result;
        for (size_t r = 0; r < tshape0; ++r) {
            for (size_t c = 0; c < shape().last(); ++c) {
                result(c, r) = (*this)(r, c);
            }
        }
        return result;
    }
    auto H() const {
        static_assert(ndim() == 2);
        FixedArray<TData, shape().last(), tshape0> result;
        for (size_t r = 0; r < tshape0; ++r) {
            for (size_t c = 0; c < shape().last(); ++c) {
                result(c, r) = conju((*this)(r, c));
            }
        }
        return result;
    }
    auto vH() const {
        return H();
    }
    FixedArray operator - () const {
        return applied([&](const TData& v){ return -v; });
    }
    const FixedArray& operator + () const {
        return *this;
    }
    template <typename TResultData>
    FixedArray<TResultData, tshape0, tshape...> casted() const {
        if constexpr (std::is_same_v<TResultData, TData>) {
            return *this;
        } else {
            return applied<TResultData>([&](const TData& v){ return (TResultData)v; });
        }
    }
    template <class Archive>
    void serialize(Archive& archive) {
        for (TData& v : flat_iterable()) {
            archive(v);
        }
    }
private:
    template <size_t i>
    void set_values() {}
    template<size_t i, typename... Values>
    void set_values(const TData& v, const Values&... values) {
        static_assert(i < nelements());
        static_assert(std::is_fundamental_v<TData>);
        *(flat_begin() + i) = v;
        // (*this)(i) = v;
        set_values<i + 1>(values...);
    }
    FixedArray<TData, tshape...> data_[tshape0];
};

template <typename TData>
class FixedArray<TData>: public BaseDenseFixedArray<FixedArray<TData>, TData>
{
public:
    typedef TData value_type;

    FixedArray() {}
    FixedArray(const TData& v)
    : value_(v)
    {}
    template <typename... Ids>
    const TData& operator() () const {
        return value_;
    }
    template <typename... Ids>
    TData& operator() () {
        return value_;
    }
    static constexpr size_t nelements() {
        return 1;
    }
    static constexpr size_t ndim() {
        return 0;
    }
    constexpr TData* flat_begin() {
        return &value_;
    }
    constexpr TData* flat_end() {
        return &value_ + 1;
    }
    constexpr const TData* flat_begin() const {
        return const_cast<FixedArray*>(this)->flat_begin();
    }
    constexpr const TData* flat_end() const {
        return const_cast<FixedArray*>(this)->flat_end();
    }
    bool less_than(const FixedArray& rhs) const {
        return value_ < rhs.value_;
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(value_);
    }
private:
    TData value_;
};

/*
 * Write to stream
 */
template <typename TData, size_t tshape0, size_t... tshape>
std::ostream& operator << (std::ostream& ostream, const FixedArray<TData, tshape0, tshape...>& a) {
    typedef FixedArray<TData, tshape0, tshape...> A;
    if constexpr (A::ndim() == 1) {
        for (size_t i = 0; i < a.length(); ++i) {
            ostream << WriteNum(a(i)) << (i == a.length() -1 ? "" : " ");
        }
    } else if constexpr (A::ndim() != 0) {
        for (size_t i = 0; i < tshape0; ++i) {
            ostream << a[i] << std::endl;
        }
    }
    return ostream;
}

}

#include <Mlib/Array/Array.hpp>

namespace Mlib {

template <typename TData, size_t tshape0, size_t... tshape>
FixedArray<TData, tshape0, tshape...>::FixedArray(const ArrayShape& shape)
: FixedArray{ shape.begin(), shape.ndim() }
{}

template <typename TData, size_t tshape0, size_t... tshape>
ArrayShape FixedArray<TData, tshape0, tshape...>::to_array_shape() const {
    static_assert(ndim() == 1);
    ArrayShape result(nelements());
    std::copy(flat_begin(), flat_end(), result.begin());
    return result;
}

template <typename TData, size_t tshape0, size_t... tshape>
ArrayShape FixedArray<TData, tshape0, tshape...>::array_shape() const {
    return ArrayShape{ tshape0, tshape... };
}

}
