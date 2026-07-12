#pragma once
#include <Mlib/Initialization/Default_Uninitialized.hpp>

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
    size_t alignment_;
public:
    inline Vector(size_t size, size_t alignment, Uninitialized)
        : size_{size}
        , alignment_{alignment == 0 ? alignof(UData) : alignment}
    {
        if ((size_ * sizeof(UData)) % alignment_ != 0) {
            throw std::runtime_error("Vector size in bytes is not a multiple of the alignment");
        }
        data_ = (UData*)std::aligned_alloc(alignment_, size * sizeof(UData));
        if (data_ == nullptr) {
            throw std::bad_alloc();
        }
        if constexpr (!std::is_trivially_constructible_v<UData>) {
            for (size_t i = 0; i < size_; ++i) {
                ::new ((void*)(&data_[i])) UData();
            }
        }
    }
    inline ~Vector() {
        if constexpr (!std::is_trivially_destructible_v<UData>) {
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~UData();
            }
        }
        std::free(data_);
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
    inline size_t alignment() const {
        return alignment_;
    }
};

}
