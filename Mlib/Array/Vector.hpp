#pragma once
#include <Mlib/Default_Uninitialized.hpp>

namespace Mlib {

/*
 * std::vector replacement to circumvent <bool> template specialization
 */
template <class TData>
class Vector {
    Vector(const Vector&) = delete;
    Vector& operator = (const Vector&) = delete;
    using UData = default_uninitialized_t<TData>;
    UData* data_;
    size_t size_;
public:
    inline explicit Vector(size_t size, Uninitialized):
        data_(new UData[size]),
        size_(size) {}
    inline ~Vector() {
        delete [] data_;
    }
    inline const TData& operator [] (size_t index) const {
        assert(index < size_);
        return data_[index];
    }
    inline TData& operator [] (size_t index) {
        const Vector& a = *this;
        return const_cast<TData&>(a[index]);
    }
    inline size_t size() const {
        return size_;
    }
};

}
