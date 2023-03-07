#pragma once
#include "Array_Forward.hpp"
#include <cassert>

namespace Mlib {

template <class TData>
class ArrayAxisView {
    Array<TData> array_;
    size_t offset_;
    size_t stride_;
    size_t length_;
public:
    ArrayAxisView(const Array<TData>& array, const ArrayShape& index0, size_t axis):
        array_(array)
    {
        assert(index0(axis) == 0);
        ArrayShape index1(index0);
        index1(axis) = 1;
        assert(array_.data_ != nullptr);
        assert(array_.data_->size() > 0);
        offset_ = (size_t)(&array_(index0) - &(*array_.data_)[0]);
        if (array_.shape(axis) > 1) {
            stride_ = (size_t)(&array_(index1) - &array_(index0));
        } else {
            stride_ = SIZE_MAX;
        }
        length_ = array_.shape(axis);
    }
    const TData& operator () (size_t index) const {
        size_t i = offset_ + stride_ * index;
        assert(index < length());
        assert(i < array_.data_->size());
        return (*array_.data_)[i];
    }
    TData& operator () (size_t index) {
        const ArrayAxisView& a = *this;
        return const_cast<TData&>(a(index));
    }
    size_t length() const {
        return length_;
    }
};

}
