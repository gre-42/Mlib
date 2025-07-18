#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Array_Shape.hpp>
#include <Mlib/Array/Array_Resizer.hpp>
#include <Mlib/Array/Base_Dense_Array.hpp>
#include <Mlib/Array/Conjugate_Transpose_Array.hpp>
#include <Mlib/Array/Range.hpp>
#include <Mlib/Array/Vector.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Conju.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Initializer_List_As_Sized_Iterable.hpp>
#include <Mlib/Io/Read_Number.hpp>
#include <Mlib/Io/Write_Number.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Io/Read_Number.hpp>
#include <Mlib/Io/Write_Number.hpp>
#include <Mlib/Uninitialized.hpp>
#include <Mlib/Iterator/Sized_Iterable.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cassert>
#include <climits>
#include <complex>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

namespace Mlib {

template <class TData>
std::ostream& operator << (std::ostream& ostream, const Array<TData>& a);
inline Array<bool> operator == (const ArrayShape& a, const ArrayShape& b);
inline Array<bool> operator != (const ArrayShape& a, const ArrayShape& b);

template <class TDerived, class TData> inline size_t count_nonzero(const BaseDenseArray<TDerived, TData>& a);
template <class TDerived> inline bool any(const BaseDenseArray<TDerived, bool>& a);
template <class TDerived> inline bool all(const BaseDenseArray<TDerived, bool>& a);

template <class TData>
Array<TData> full(const ArrayShape& shape, const TData& value);

template <typename TData, size_t... tshape>
class FixedArray;

template <class TData>
class ArrayIterator {
public:
    explicit ArrayIterator(size_t row, const Array<TData>& array)
    : row_(row), array_(array) {}

    Array<TData> operator * () const {
        return array_[row_];
    }

    ArrayIterator& operator ++() {
        ++row_;
        return *this;
    }

    bool operator == (const ArrayIterator<TData>& other) const {
        return row_ == other.row_;
    }

    bool operator != (const ArrayIterator<TData>& other) const {
        return row_ != other.row_;
    }
private:
    size_t row_;
    Array<TData> array_;
};

template <class TData>
class ConstArrayIterator {
public:
    explicit ConstArrayIterator(size_t row, const Array<TData>& array)
    : row_(row), array_(array) {}

    const Array<TData> operator * () const {
        return array_[row_];
    }

    const ConstArrayIterator& operator ++() {
        ++row_;
        return *this;
    }

    bool operator == (const ConstArrayIterator<TData>& other) const {
        return row_ == other.row_;
    }

    bool operator != (const ConstArrayIterator<TData>& other) const {
        return row_ != other.row_;
    }
private:
    size_t row_;
    const Array<TData> array_;
};

template <class TData>
class ArrayElementIterable {
public:
    explicit ArrayElementIterable(Array<TData>& array)
    : array_(array) {}

    TData* begin() {
        return (array_.length() == 0) ? nullptr : &array_(0);
    }

    TData* end() {
        return (array_.length() == 0) ? nullptr : (&array_(0) + array_.length());
    }
private:
    Array<TData> array_;
};

template <class TData>
class ConstArrayElementIterable {
public:
    explicit ConstArrayElementIterable(const Array<TData>& array)
    : array_(array) {}

    const TData* begin() const {
        return (array_.length() == 0) ? nullptr : &array_(0);
    }

    const TData* end() const {
        return (array_.length() == 0) ? nullptr : (&array_(0) + array_.length());
    }
private:
    const Array<TData> array_;
};

// template <class TData>
// inline void fill_with_invalid_value(Array<TData>& a);

template <class TData>
class ArrayRefAssigner {
public:
    explicit ArrayRefAssigner(Array<TData>& ar)
    : ar_{ar}
    {}
    Array<TData>& operator = (const Array<TData>& rhs) {
        ar_.data_ = rhs.data_;
        ar_.shape_ = rhs.shape_;
        ar_.offset_ = rhs.offset_;
        return ar_;
    }
private:
    Array<TData>& ar_;
};

template <class TData>
class ArrayMoveAssigner {
public:
    explicit ArrayMoveAssigner(Array<TData>& ar)
    : ar_{ar}
    {}
    Array<TData>& operator = (Array<TData>&& rhs) {
        ar_.data_ = std::move(rhs.data_);
        ar_.shape_ = std::move(rhs.shape_);
        ar_.offset_ = std::move(rhs.offset_);
        return ar_;
    }
private:
    Array<TData>& ar_;
};

template <class TContainer, class TData>
concept DataSizedIterable = requires()
{
    requires SizedIterable<TContainer>;
    requires std::same_as<typename TContainer::value_type, TData>;
};

template <class TContainer, class TData>
concept ArraySizedIterable = requires()
{
    requires SizedIterable<TContainer>;
    requires std::same_as<typename TContainer::value_type, Array<TData>>;
};

template <class TContainer, class TData, size_t ...tsize>
concept FixedArraySizedSizedIterable = requires()
{
    requires SizedIterable<TContainer>;
    requires std::same_as<typename TContainer::value_type, FixedArray<TData, tsize...>>;
};

template <class TData>
class Array: public BaseDenseArray<Array<TData>, TData> {
    friend ArrayAxisView<TData>;
    friend ArrayRefAssigner<TData>;
    friend ArrayMoveAssigner<TData>;
    std::shared_ptr<Vector<TData>> data_;
    std::shared_ptr<ArrayShape> shape_;
    size_t offset_;
public:
    typedef TData value_type;
    ArrayResizer resize;
    ArrayResizer reshape;
    Array(Uninitialized)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {}
    Array()
        : Array(uninitialized)
    {}
    Array(const Array& rhs)
        : data_{ rhs.data_ }
        , shape_{ rhs.shape_ }
        , offset_{ rhs.offset_ }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {}
    Array(Array&& rhs) noexcept
        : data_{ std::move(rhs.data_) }
        , shape_{ std::move(rhs.shape_) }
        , offset_{ rhs.offset_ }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {}
    template <ArraySizedIterable<TData> TArrayContainer>
    explicit Array(
        const TArrayContainer& rhs,
        const ArrayShape& empty_shape=ArrayShape())
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        // Input is a list => empty_shape must be at least 1D
        if ((rhs.size() == 0) && (empty_shape.ndim() == 0)) {
            // Not an assertion because it is used for file-io
            THROW_OR_ABORT("Cannot construct an empty array from a list without empty_shape parameter");
        }
        if (rhs.size() == 0) {
            do_resize(empty_shape);
        } else {
            do_resize(ArrayShape{ rhs.size() }.concatenated(rhs.begin()->shape()));
            auto it = rhs.begin();
            for (size_t i = 0; i < rhs.size(); ++i) {
                if (any(it->shape() != rhs.begin()->shape())) {
                    // Not an assertion because it is used for file-io
                    THROW_OR_ABORT("Arrays in lists have differing sizes");
                }
                (*this)[i] = *(it++);
            }
        }
    }
    explicit Array(const TData* begin, const TData* end)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        size_t count = size_t(end - begin);
        do_resize(ArrayShape{count});
        if (count > 0) {
            std::copy(begin, end, &(*this)(0));
        }
    }
    template <DataSizedIterable<TData> TDataContainer>
    explicit Array(const TDataContainer& lst)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        do_resize(ArrayShape{ lst.size() });
        size_t i = 0;
        for (const auto& value : lst) {
            (*this)(i) = value;
            ++i;
        }
    }
    template <size_t ...tsize, FixedArraySizedSizedIterable<TData, tsize...> TDataContainer>
    explicit Array(const TDataContainer& lst)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        constexpr size_t nelements = FixedArray<TData, tsize...>::nelements();
        do_resize(ArrayShape{ lst.size(), nelements });
        size_t i = 0;
        for (const auto& value : lst) {
            const auto& flat_value = reinterpret_cast<const FixedArray<TData, nelements>&>(value);
            for (size_t j = 0; j < nelements; ++j) {
                (*this)(i, j) = flat_value(j);
            }
            ++i;
        }
        do_reshape(ArrayShape{ lst.size(), tsize... });
    }
    template <size_t ...tsize>
    explicit Array(const Array<FixedArray<TData, tsize...>>& rhs)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        do_resize(ArrayShape{ rhs.nelements(), FixedArray<TData, tsize...>::nelements() });
        auto rhs_flat = rhs.flattened();
        auto& rhs_flat2 = reinterpret_cast<const Array<FixedArray<TData, FixedArray<TData, tsize...>::nelements()>>&>(rhs_flat);
        for (size_t i = 0; i < shape(0); ++i) {
            for (size_t j = 0; j < shape(1); ++j) {
                (*this)(i, j) = rhs_flat2(i)(j);
            }
        }
        do_reshape(rhs.shape().concatenated(ArrayShape{ tsize... }));
    }
    template <size_t ...tsize>
    static Array<FixedArray<TData, tsize...>> from_dynamic(const Array<TData>& rhs) {
        constexpr static const size_t flat_elements = FixedArray<TData, tsize...>::nelements();
        constexpr static const size_t static_ndim = FixedArray<TData, tsize...>::ndim();
        assert(rhs.ndim() >= static_ndim);
        assert(all(FixedArray<size_t, static_ndim>{tsize...} == FixedArray<size_t, static_ndim>{rhs.shape().erased_first(rhs.ndim() - static_ndim)}));
        ArrayShape result_shape{ rhs.shape().erased_last(static_ndim) };
        Array<FixedArray<TData, tsize...>> result{ ArrayShape{result_shape.nelements()} };
        if (result.length() > 0) {
            Array<TData> rhs_flat = rhs.reshaped(ArrayShape{ result.length(), rhs.nelements() / result.length() });
            auto& flat_result = reinterpret_cast<Array<FixedArray<TData, flat_elements>>&>(result);
            for (size_t i = 0; i < result.length(); ++i) {
                for (size_t j = 0; j < flat_elements; ++j) {
                    flat_result(i)(j) = rhs_flat(i, j);
                }
            }
        }
        result.reshape(result_shape);
        return result;
    }
    explicit Array(std::initializer_list<TData> d)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        do_resize(ArrayShape{ d.size() });
        size_t i = 0;
        for (const auto& value : d) {
            (*this)(i) = value;
            ++i;
        }
    }
    explicit Array(std::initializer_list<std::initializer_list<TData>> d)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        assert(d.size() > 0);
        size_t ncols = d.begin()->size();
        do_resize(ArrayShape{ d.size(), ncols });
        size_t r = 0;
        for (const auto& row : d) {
            assert(row.size() == ncols);
            size_t c = 0;
            for (const auto& value : row) {
                (*this)(r, c) = value;
                ++c;
            }
            ++r;
        }
    }
    explicit Array(std::initializer_list<Array<TData>> d)
        : Array(InitializerListAsSizedIterable(d))
    {
        if (d.size() == 1) {
            THROW_OR_ABORT("Do not use single initializer for arrays");
        }
    }
    explicit Array(const ArrayShape& shape)
        : offset_{ 0 }
        , resize{ [this](const ArrayShape& shape) { do_resize(shape); } }
        , reshape{ [this](const ArrayShape& shape) { do_reshape(shape); } }
    {
        do_resize(shape);
    }
    ~Array() {}
    template <class TDerived>
    Array& operator = (const BaseDenseArray<TDerived, TData>& rhs) {
        if (!rhs->initialized()) {
            destroy();
        } else {
            if (shape_ == nullptr) {
                do_resize(rhs->shape());
            }
            assert(all(shape() == rhs->shape()));
            if (ndim() == 0) {
                (*this)() = (*rhs)();
            } else if (ndim() == 1) {
                for (size_t i=0; i<length(); i++) {
                    (*this)(i) = (*rhs)(i);
                }
            } else {
                Array f = flattened();
                Array g = rhs->flattened();
                f = g;
            }
        }
        return *this;
    }
    template <size_t... tshape>
    Array& operator = (const FixedArray<TData, tshape...>& rhs) {
        if (shape_ == nullptr) {
            do_resize(ArrayShape{ tshape... });
        }
        assert(all(shape() == ArrayShape{ tshape... }));
        auto rit = rhs.flat_begin();
        for (auto& lhs : flat_iterable()) {
            lhs = *rit++;
        }
        return *this;
    }
    Array& operator = (const Array& rhs) {
        const BaseDenseArray<Array<TData>, TData>& b_rhs = rhs;
        *this = b_rhs;
        return *this;
    }
    ArrayRefAssigner<TData> ref() {
        return ArrayRefAssigner{ *this };
    }
    ArrayMoveAssigner<TData> move() {
        return ArrayMoveAssigner{ *this };
    }
    Array& operator = (const TData& rhs) {
        Array f = flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) = rhs;
        }
        return *this;
    }
    Array& operator &= (const Array& rhs) {
        assert(all(shape() == rhs.shape()));
        Array f = flattened();
        Array g = rhs.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) &= g(i);
        }
        return *this;
    }
    Array& operator |= (const Array& rhs) {
        assert(all(shape() == rhs.shape()));
        Array f = flattened();
        Array g = rhs.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) |= g(i);
        }
        return *this;
    }
    Array<bool> operator == (const Array &rhs) const {
        assert(all(shape() == rhs.shape()));
        Array<bool> result{shape()};
        Array f = flattened();
        Array g = rhs.flattened();
        Array<bool> e = result.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            e(i) = (f(i) == g(i));
        }
        return result;
    }
    Array<bool> operator != (const Array &rhs) const {
        assert(all(shape() == rhs.shape()));
        Array<bool> result{shape()};
        Array f = flattened();
        Array g = rhs.flattened();
        Array<bool> e = result.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            e(i) = (f(i) != g(i));
        }
        return result;
    }
    /**
     * Multidimensional mask-access, involves flattening.
     */
    Array operator [] (const Array<bool>& mask) const {
        assert(all(shape() == mask.shape()));
        Array f = flattened();
        Array<bool> m = mask.flattened();
        Array result(ArrayShape{count_nonzero(m)});
        size_t j = 0;
        for (size_t i = 0; i < f.length();  ++i) {
            if (m(i)) {
                result(j) = f(i);
                ++j;
            }
        }
        return result;
    }
    /**
     * Row-wise index-access.
     */
    Array operator [] (const Array<size_t>& indices) const {
        assert(indices.ndim() == 1);
        Array result(ArrayShape{indices.length()}.concatenated(shape().erased_first()));
        if (ndim() == 1) {
            for (size_t i = 0; i < indices.length();  ++i) {
                result(i) = (*this)(indices(i));
            }
        } else {
            for (size_t i = 0; i < indices.length();  ++i) {
                result[i] = (*this)[indices(i)];
            }
        }
        return result;
    }
    // 1-ND slice
    Array operator [] (size_t index) const {
        assert(ndim() > 0);
        assert(shape(0) > index);
        Array result = *this;
        result.do_reshape(this->shape().erased_first());
        result.offset_ += result.shape().nelements() * index;
        return result;
    }
    // 1D access
    inline const TData& operator () (size_t index) const {
        assert(ndim() == 1);
        assert(shape(0) > index);
        assert(index < length());
        assert(index + offset_ < data_->size());
        return (*data_)[index + offset_];
    }
    inline TData& operator () (size_t index) {
        const Array& a = *this;
        return const_cast<TData&>(a(index));
    }
    // 2D access
    inline const TData& operator () (size_t i, size_t j) const {
        assert(ndim() == 2);
        assert(shape(0) > i);
        assert(shape(1) > j);
        size_t index = i * shape(1) + j;
        assert(index + offset_ < data_->size());
        return (*data_)[index + offset_];
    }
    inline TData& operator () (size_t i, size_t j) {
        const Array& a = *this;
        return const_cast<TData&>(a(i, j));
    }
    // 3D access
    inline const TData& operator () (size_t i, size_t j, size_t k) const {
        assert(ndim() == 3);
        assert(shape(0) > i);
        assert(shape(1) > j);
        assert(shape(2) > k);
        size_t index = (i * shape(1) + j) * shape(2) + k;
        assert(index + offset_ < data_->size());
        return (*data_)[index + offset_];
    }
    inline TData& operator () (size_t i, size_t j, size_t k) {
        const Array& a = *this;
        return const_cast<TData&>(a(i, j, k));
    }
    // 4D access
    inline const TData& operator () (size_t i, size_t j, size_t k, size_t n) const {
        assert(ndim() == 4);
        assert(shape(0) > i);
        assert(shape(1) > j);
        assert(shape(2) > k);
        assert(shape(3) > n);
        size_t index = ((i * shape(1) + j) * shape(2) + k) * shape(3) + n;
        assert(index + offset_ < data_->size());
        return (*data_)[index + offset_];
    }
    inline TData& operator () (size_t i, size_t j, size_t k, size_t n) {
        const Array& a = *this;
        return const_cast<TData&>(a(i, j, k, n));
    }
    // 0D access
    inline const TData& operator () () const {
        assert(ndim() == 0);
        assert(offset_ < data_->size());
        return (*data_)[offset_];
    }
    inline TData& operator () () {
        const Array& a = *this;
        return const_cast<TData&>(a());
    }
    // ND access
    Array operator [] (const ArrayShape& index) const {
        assert(ndim() >= index.ndim());
        if (index.ndim() == 0) {
            return (*this);
        } else {
            return (*this)[index(0)][index.erased_first()];
        }
    }
    inline const TData& operator () (const ArrayShape& index) const {
        assert(ndim() == index.ndim());
        if (index.ndim() == 0) {
            return (*this)();
        }
        else {
            return (*this)[index(0)](index.erased_first());
        }
    }
    inline TData& operator () (const ArrayShape& index) {
        const Array& a = *this;
        return const_cast<TData&>(a(index));
    }
    inline const TData& operator () (const FixedArray<size_t, 2>& index) const {
        assert(ndim() == index.length());
        return (*this)(index(0), index(1));
    }
    inline const TData& operator () (const FixedArray<size_t, 3>& index) const {
        assert(ndim() == index.length());
        return (*this)(index(0), index(1), index(2));
    }
    inline const TData& operator () (const FixedArray<size_t, 4>& index) const {
        assert(ndim() == index.length());
        return (*this)(index(0), index(1), index(2), index(3));
    }
    template <size_t tsize>
    inline TData& operator () (const FixedArray<size_t, tsize>& index) {
        const Array& a = *this;
        return const_cast<TData&>(a(index));
    }
    // Shape
    inline const ArrayShape &shape() const {
        assert(shape_ != nullptr);
        return *shape_;
    }
    inline const ArrayShape& array_shape() const {
        return shape();
    }
    template <size_t tndim>
    const FixedArray<size_t, tndim> fixed_shape() const {
        if (ndim() != tndim) {
            THROW_OR_ABORT("fixed_shape of incorrect size requested");
        }
        return FixedArray<size_t, tndim>::from_buffer(shape().begin(), ndim());
    }
    inline size_t shape(size_t i) const {
        return shape()(i);
    }
    template <size_t N>
    inline size_t static_shape() const {
        return shape(N);
    }
    inline size_t ndim() const {
        return shape().ndim();
    }
    size_t nelements() const {
        // not correct, truncated elements after current slice are missing
        // (would require a second offset counting from the end)
        // assert(shape().nelements() == data_->shape() - offset_);
        return shape().nelements();
    }
    size_t nbytes() const {
        return nelements() * sizeof(TData);
    }
    size_t length() const {
        return shape().length();
    }
    inline bool initialized() const {
        // See also: destroy()
        return shape_ != nullptr;
    }
    void destroy() {
        // See also: initialized()
        data_ = nullptr;
        shape_ = nullptr;
        offset_ = 0;
    }
    Array& reassign(const Array& other) {
        assert(initialized());
        destroy();
        *this = other;
        return *this;
    }
    Array reshaped(const ArrayShape& shape) const {
        Array result = *this;
        result.do_reshape(shape);
        return result;
    }
    Array flattened() const {
        Array result = *this;
        result.do_reshape(ArrayShape{nelements()});
        return result;
    }
    void rows_to_1D() {
        do_reshape(ArrayShape{
            shape().erased_last().nelements(),
            shape(ndim() - 1) });
    }
    void columns_to_1D() {
        do_reshape(ArrayShape{
            shape(0),
            shape().erased_first().nelements() });
    }
    Array rows_as_1D() const {
        Array result = *this;
        result.rows_to_1D();
        return result;
    }
    Array columns_as_1D() const {
        Array result = *this;
        result.columns_to_1D();
        return result;
    }
    void to_row_vector() {
        assert(ndim() == 1);
        do_reshape(ArrayShape{1, length()});
    }
    Array as_row_vector() const {
        Array result = *this;
        result.to_row_vector();
        return result;
    }
    void to_column_vector() {
        assert(ndim() == 1);
        do_reshape(ArrayShape{length(), 1});
    }
    Array as_column_vector() const {
        Array result = *this;
        result.to_column_vector();
        return result;
    }
    Array T(size_t block_size=1) const {
        if (ndim() <= 1) {
            return copy();
        } else if (ndim() == 2) {
            Array result{ArrayShape{shape(1), shape(0)}};
            if (block_size == 0) {
                THROW_OR_ABORT("Block size must be >= 1");
            }
            if (block_size == 1) {
                for (size_t r = 0; r < shape(0); ++r) {
                    for (size_t c = 0; c < shape(1); ++c) {
                        result(c, r) = (*this)(r, c);
                    }
                }
            } else {
                for (size_t i = 0; i < shape(0); i += block_size) {
                    for (size_t j = 0; j < shape(1); j += block_size) {
                        for (size_t r = i; r < std::min(i + block_size, shape(0)); ++r) {
                            for (size_t c = j; c < std::min(j + block_size, shape(1)); ++c) {
                                result(c, r) = (*this)(r, c);
                            }
                        }
                    }
                }
            }
            return result;
        } else {
            lerr() << "Warning: using slow transposition algorithm";
            Array result;
            result.do_resize(shape().reverted());
            shape().foreach([&](const ArrayShape& i) {
                result(i.reverted()) = (*this)(i);
            });
            return result;
        }
    }
    Array H() const {
        return T().applied([](const TData& v){ return conju(v); });
    }
    ConjugateTransposeArray<TData> vH() const {
        return ConjugateTransposeArray(*this);
    }
    Array& operator += (const Array& rhs) {
        assert(all(shape() == rhs.shape()));
        Array f = flattened();
        Array g = rhs.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) += g(i);
        }
        return *this;
    }
    Array& operator -= (const Array& rhs) {
        assert(all(shape() == rhs.shape()));
        Array f = flattened();
        Array g = rhs.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) -= g(i);
        }
        return *this;
    }
    Array& operator /= (const Array& rhs) {
        assert(all(shape() == rhs.shape()));
        Array f = flattened();
        Array g = rhs.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) /= g(i);
        }
        return *this;
    }
    Array& operator *= (const Array& rhs) {
        assert(all(shape() == rhs.shape()));
        Array f = flattened();
        Array g = rhs.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) *= g(i);
        }
        return *this;
    }
    Array& operator -= (const TData& rhs) {
        Array f = flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) -= rhs;
        }
        return *this;
    }
    Array& operator += (const TData& rhs) {
        Array f = flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) += rhs;
        }
        return *this;
    }
    Array& operator /= (const TData& rhs) {
        Array f = flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) /= rhs;
        }
        return *this;
    }
    Array& operator *= (const TData& rhs) {
        Array f = flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            f(i) *= rhs;
        }
        return *this;
    }
    Array operator - () const {
        Array result;
        result.do_resize(shape());
        Array f = flattened();
        Array g = result.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            g(i) = -f(i);
        }
        return result;
    }
    Array copy() const {
        Array result;
        result = *this;
        return result;
    }
    template <class TNewData>
    Array<TNewData> casted() const {
        Array<TNewData> result;
        result.do_resize(shape());
        Array f = flattened();
        Array<TNewData> g = result.flattened();
        for (size_t i = 0; i < f.length();  ++i) {
            g(i) = static_cast<TNewData>(f(i));
        }
        return result;
    }
    Array reversed() const {
        Array result(shape());
        if (ndim() == 1) {
            for (size_t r = 0; r < length(); ++r) {
                result(length() - 1 - r) = (*this)(r);
            }
        } else if (ndim() != 0) {
            for (size_t r = 0; r < shape(0); ++r) {
                result[shape(0) - 1 - r] = (*this)[r];
            }
        }
        return result;
    }
    Array reversed(size_t axis) const {
        return apply_over_axis(axis, ApplyOverAxisType::NOREDUCE,
            [&, this](size_t i, size_t k, const Array<TData>& xf, Array<TData>& rf)
            {
                for (size_t h = 0; h < shape(axis); ++h) {
                    rf(i, h, k) = xf(i, shape(axis) - 1 - h, k);
                }
            });
    }
    std::string str() const {
        std::stringstream sstr;
        sstr << *this;
        return sstr.str();
    }
    Array row_range(size_t begin, size_t end) const {
        assert(end >= begin);
        assert(ndim() > 0);
        assert((begin == end) || (shape(0) > begin));
        Array result = *this;
        result.do_reshape(this->shape().erased_first());
        result.offset_ += result.shape().nelements() * begin;
        result.do_reshape((ArrayShape{size_t(end - begin)}.concatenated(shape().erased_first())));
        return result;
    }

    Array col_range(size_t begin, size_t end) const {
        return T().row_range(begin, end).T();
    }

    Array blocked(const Array<size_t>& row_ids, const Array<size_t>& col_ids) const {
        assert(ndim() == 2);
        Array res{ArrayShape{row_ids.length(), col_ids.length()}};
        for (size_t r = 0; r < res.shape(0); ++r) {
            for (size_t c = 0; c < res.shape(1); ++c) {
                res(r, c) = (*this)(row_ids(r), col_ids(c));
            }
        }
        return res;
    }

    Array blocked(const Range& rows, const Range& cols) const {
        assert(ndim() == 2);
        assert(rows.end >= rows.begin);
        assert(cols.end >= cols.begin);
        Array res{ArrayShape{rows.length(), cols.length()}};
        for (size_t r = 0; r < res.shape(0); ++r) {
            for (size_t c = 0; c < res.shape(1); ++c) {
                res(r, c) = (*this)(rows.begin + r, cols.begin + c);
            }
        }
        return res;
    }

    Array blocked(const Array<size_t>& row_ids) const {
        assert(ndim() == 1);
        return as_column_vector().blocked(row_ids, Array<size_t>{0}).flattened();
    }

    Array unblocked(const Array<size_t>& row_ids, const Array<size_t>& col_ids, const ArrayShape& unblocked_shape, const float& fill_value = NAN) const {
        assert(ndim() == 2);
        assert(row_ids.length() == shape(0));
        assert(col_ids.length() == shape(1));
        Array res = full<TData>(unblocked_shape, fill_value);
        for (size_t r = 0; r < shape(0); ++r) {
            for (size_t c = 0; c < shape(1); ++c) {
                res(row_ids(r), col_ids(c)) = (*this)(r, c);
            }
        }
        return res;
    }

    Array unblocked(const Array<size_t>& indices, size_t unblocked_length, const float& fill_value = NAN) const {
        assert(ndim() == 1);
        assert(indices.length() == length());
        Array res = full<TData>(ArrayShape{unblocked_length}, fill_value);
        for (size_t i = 0; i < length(); ++i) {
            res(indices(i)) = (*this)(i);
        }
        return res;
    }

    Array masked(const Array<size_t>& row_ids, const Array<size_t>& col_ids, const float& fill_value = NAN) {
        return blocked(row_ids, col_ids).unblocked(row_ids, col_ids, shape(), fill_value);
    }

    Array masked(const Array<size_t>& row_ids, const float& fill_value = NAN) {
        return blocked(row_ids).unblocked(row_ids, length(), fill_value);
    }

    void save_txt_2d(const std::string& filename) const {
        assert(ndim() == 2);
        std::ofstream ofs(filename);
        if (ofs.fail()) {
            THROW_OR_ABORT("Could not open file \"" + filename + '"');
        }
        ofs.setf(std::ios_base::scientific);
        ofs.precision(10);
        for (size_t r = 0; r < shape(0); ++r) {
            for (size_t c = 0; c < shape(1); ++c) {
                ofs << WriteNum((*this)(r, c)) << ((c != shape(1) - 1) ? " " : "");
            }
            ofs << '\n';
        }
        ofs.flush();
        if (ofs.fail()) {
            THROW_OR_ABORT("Could not save to file \"" + filename + '"');
        }
    }

    static Array load_txt_2d(
        const std::string& filename,
        const ArrayShape& empty_shape = ArrayShape())
    {
        std::list<Array> result;
        auto ifs_p = create_ifstream(filename);
        auto& ifs = *ifs_p;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not open file \"" + filename + '"');
        }
        std::string line;
        while(std::getline(ifs, line)) {
            std::vector<TData> rowv;
            std::stringstream srow(line);
            TData value;
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            while(srow >> ReadNum{value}) {
                rowv.push_back(value);
            }
            #pragma GCC diagnostic pop
            if (srow.fail() && !srow.eof()) {
                THROW_OR_ABORT("Could not read line of file \"" + filename + '"');
            }
            Array arow(ArrayShape{rowv.size()});
            for (size_t c = 0; c < arow.length(); ++c) {
                arow(c) = rowv[c];
            }
            result.push_back(arow);
        }
        if (ifs.fail() && !ifs.eof()) {
            THROW_OR_ABORT("Could not read line of file \"" + filename + '"');
        }
        return Array(result, empty_shape);
    }

    void save_binary(const std::string& filename) const {
        std::ofstream ofs(filename, std::ios::binary);
        if (ofs.fail()) {
            THROW_OR_ABORT("Could not open file \"" + filename + '"');
        }
        ofs << "BinaryArray\n" << ndim();
        for (size_t i = 0; i < ndim(); ++i) {
            ofs << " " << shape(i);
        }
        ofs << '\n' << sizeof(TData) << '\n';
        ofs.write((const char*)flat_iterable().begin(), integral_cast<std::streamsize>(nbytes()));
        ofs.flush();
        if (ofs.fail()) {
            THROW_OR_ABORT("Could not save to file \"" + filename + '"');
        }
    }

    static Array load_binary(const std::string& filename) {
        auto ifs_p = create_ifstream(filename, std::ios::binary);
        auto& ifs = *ifs_p;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not open file \"" + filename + '"');
        }
        std::string first_line;
        ifs >> first_line;
        if (first_line != "BinaryArray") {
            THROW_OR_ABORT("File \"" + filename + "\" has no first line \"BinaryArray\"");
        }
        size_t ndim;
        ifs >> ndim;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not read ndim from file \"" + filename + '"');
        }
        ArrayShape s(ndim);
        for (size_t i = 0; i < ndim; ++i) {
            ifs >> s(i);
        }
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not read shape from file \"" + filename + '"');
        }
        size_t element_size;
        ifs >> element_size;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not element size from file \"" + filename + '"');
        }
        if (element_size != sizeof(TData)) {
            THROW_OR_ABORT("Wrong element size in file \"" + filename + '"');
        }
        auto c = read_binary<char>(ifs, "newline character", IoVerbosity::SILENT);
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not read newline-character of file \"" + filename + '"');
        }
        if (c != '\n') {
            THROW_OR_ABORT("Did not find newline-character in file \"" + filename + '"');
        }
        Array res{s};
        ifs.read((char*)res.flat_iterable().begin(), integral_cast<std::streamsize>(res.nbytes()));
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not read data from file \"" + filename + '"');
        }
        return res;
    }

    static Array from_shape(const ArrayShape& shape) {
        Array result(ArrayShape{shape.ndim()});
        for (size_t d = 0; d < shape.ndim(); ++d) {
            result(d) = (TData)shape(d);
        }
        return result;
    }
    ArrayShape to_shape() const {
        assert(ndim() == 1);
        ArrayShape result(length());
        for (size_t d = 0; d < length(); ++d) {
            result(d) = (*this)(d);
        }
        return result;
    }
    std::vector<TData> to_vector() const {
        assert(ndim() == 1);
        return std::vector<TData>{flat_iterable().begin(), flat_iterable().end()};
    }
    ArrayIterator<TData> begin() {
        return ArrayIterator<TData>(0, *this);
    }
    ArrayIterator<TData> end() {
        return ArrayIterator<TData>(shape(0), *this);
    }
    ConstArrayIterator<TData> begin() const {
        return ConstArrayIterator<TData>(0, *this);
    }
    ConstArrayIterator<TData> end() const {
        return ConstArrayIterator<TData>(shape(0), *this);
    }
    ArrayElementIterable<TData> element_iterable() {
        return ArrayElementIterable(*this);
    }
    ConstArrayElementIterable<TData> element_iterable() const {
        return ConstArrayElementIterable(*this);
    }
    ArrayElementIterable<TData> flat_iterable() {
        auto f = flattened();
        return ArrayElementIterable(f);
    }
    ConstArrayElementIterable<TData> flat_iterable() const {
        return ConstArrayElementIterable(flattened());
    }
    TData* flat_begin() {
        return flat_iterable().begin();
    }
    const TData* flat_begin() const {
        return flat_iterable().begin();
    }
    TData* flat_end() {
        return flat_iterable().end();
    }
    const TData* flat_end() const {
        return flat_iterable().end();
    }
    void do_resize(const ArrayShape& shape) {
        assert(offset_ == 0);
        data_ = std::make_shared<Vector<TData>>(shape.nelements(), uninitialized);
        shape_ = std::make_shared<ArrayShape>();
        *shape_ = shape;
        // offset_ = 0;
        // fill_with_invalid_value(*this);
    }
    void do_reshape(const ArrayShape& shape) {
        shape_ = std::make_shared<ArrayShape>();
        *shape_ = shape;
    }
    template <class TResultData=TData, class TOperation>
    Array<TResultData> applied(const TOperation &operation) const {
        Array<TResultData> r{shape()};
        Array af = flattened();
        Array<TResultData> rf = r.flattened();
        if (rf.length() > INT_MAX) {
            THROW_OR_ABORT("Vector too long");
        }
        int len = (int)rf.length();
        #pragma omp parallel for if (len > 25)
        for (int i = 0; i < len; ++i) {
            rf((size_t)i) = operation(af((size_t)i));
        }
        return r;
    }
    template <class TResultData=TData, class TOperation>
    Array<TResultData> apply_over_axis(
        size_t axis,
        ApplyOverAxisType aoatype,
        const TOperation& op) const
    {
        // Sums over an axis by converting
        // x(i, j, h, k, l) to x(i * j, h, k * l)
        assert(axis < ndim());
        ArrayShape shape_left;
        for (size_t i = 0; i < axis; ++i) {
            shape_left.append(shape(i));
        }
        ArrayShape shape_right;
        for (size_t i = axis + 1; i != ndim(); ++i) {
            shape_right.append(shape(i));
        }

        const Array xf = reshaped(
            ArrayShape{
                shape_left.nelements(),
                shape(axis),
                shape_right.nelements()});
        Array<TResultData> rf{aoatype == ApplyOverAxisType::REDUCE
            ? ArrayShape{
                shape_left.nelements(),
                shape_right.nelements()}
            : xf.shape()};
        for (size_t i = 0; i < xf.shape(0); ++i) {
            for (size_t k = 0; k < xf.shape(2); ++k) {
                op(i, k, xf, rf);
            }
        }
        return rf.reshaped(aoatype == ApplyOverAxisType::REDUCE
            ? shape_left.concatenated(shape_right)
            : shape());
    }
    template <class TDataResult=TData, class TDataB, class TBinop>
    Array<TDataResult> array_array_binop(const Array<TDataB>& b, const TBinop &binop) const {
        const Array& a = *this;
        assert(all(a.shape() == b.shape()));
        Array<TDataResult> r{a.shape()};
        Array af = a.flattened();
        Array<TDataB> bf = b.flattened();
        Array<TDataResult> rf = r.flattened();
        if (rf.length() > INT_MAX) {
            THROW_OR_ABORT("Vector too long");
        }
        int len = (int)rf.length();
        #pragma omp parallel for if (len > 25)
        for (int i = 0; i < len; ++i) {
            rf((size_t)i) = binop(af((size_t)i), bf((size_t)i));
        }
        return r;
    }
    template <class TResultData>
    Array<TResultData> take(
        const Array<TResultData>& values,
        const TData& nan_id = SIZE_MAX,
        const TResultData& nan_value = NAN) const
    {
        Array<TResultData> res{ArrayShape{nelements()}};
        Array xf = flattened();
        for (size_t i = 0; i < res.length(); ++i) {
            if (xf(i) == nan_id) {
                res(i) = nan_value;
            } else {
                res(i) = values(xf(i));
            }
        }
        return res.reshaped(shape());
    }
    template <class TDerived>
    Array appended(const BaseDenseArray<TDerived, TData>& rhs) const {
        Array res{ArrayShape{length() + rhs->length()}};
        res.row_range(0, length()) = *this;
        res.row_range(length(), res.length()) = *rhs;
        return res;
    }
    template <class TDerived>
    void append(const BaseDenseArray<TDerived, TData>& rhs) {
        this->move() = appended(rhs);
    }
    void append(const TData& value) {
        assert(initialized());
        assert(ndim() == 1);
        if (length() + offset_ < data_->size()) {
            do_reshape(ArrayShape{ length() + 1 });
        } else {
            size_t old_length = length();
            Array n{ArrayShape{(length() + 1) * 2}};
            n.row_range(0, length()) = *this;
            reassign(n);
            do_reshape(ArrayShape{ old_length + 1 });
        }
        (*this)(length()-1) = value;
    }
};

/*
 * Write to stream
 */
template <class TData>
std::ostream& operator << (std::ostream& ostream, const Array<TData>& a) {
    if (!a.initialized()) {
        ostream << "<uninitialized array>";
    } else if (a.ndim() == 1) {
        for (size_t i = 0; i < a.length(); ++i) {
            ostream << a(i) << (i == a.length() -1 ? "" : " ");
        }
    } else if (a.ndim() != 0) {
        for (size_t i = 0; i < a.shape(0); ++i) {
            ostream << a[i] << std::endl;
        }
    }
    return ostream;
}

// template <class TData>
// inline void fill_with_invalid_value(Array<TData>& a) {}
//
// template <>
// inline void fill_with_invalid_value<float>(Array<float>& a) {
//     a = SNAN;
// }
//
// template <>
// inline void fill_with_invalid_value<double>(Array<double>& a) {
//     a = SNAN;
// }

}

#include "Array_Axis_View.hpp" // only included for end user convenience
#include <Mlib/Math/Math.hpp>
