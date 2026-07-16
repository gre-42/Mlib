#pragma once
#include <Mlib/Initialization/Default_Uninitialized.hpp>
#include <Mlib/Memory/Aligned_Alloc.hpp>
#include <cstdlib>
#include <sstream>

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
    inline Vector(size_t size, size_t alignment, Uninitialized)
        : size_{size}
    {
        if (alignment % alignof(TData) != 0) {
            throw std::runtime_error((std::stringstream() << "Alignment is not a multiple of 'alignof'. " <<
                "Alignment (must be a power of 2): " << alignment <<
                ", alignof: " << alignof(UData)).str());
        }
        if ((size_ * sizeof(UData)) % alignment != 0) {
            throw std::runtime_error((std::stringstream() << "Vector size in bytes is not a multiple of the alignment. " <<
                "Alignment (must be a power of 2): " << alignment <<
                ", count: " << size <<
                ", element size: " << sizeof(UData)).str());
        }
#ifdef __EMSCRIPTEN__
        // Avoid infinite loop when replacing "new", by calling the low-level allocator
        if (int result = posix_memalign((void**)&data_, alignment, size * sizeof(UData)); result != 0) {
            data_ = nullptr;
        }
#else
        data_ = (UData*)::Mlib::aligned_alloc(alignment, size * sizeof(UData));
#endif
        if (data_ == nullptr) {
            throw std::runtime_error((std::stringstream() << "Memory allocation failed. " <<
                "Alignment (must be a power of 2): " << alignment <<
                ", count: " << size <<
                ", element size: " << sizeof(UData)).str());
        }
        if constexpr (!std::is_trivially_constructible_v<UData>) {
            for (size_t i = 0; i < size_; ++i) {
                ::new (data_ + i) UData();
            }
        }
    }
    inline ~Vector() {
        if constexpr (!std::is_trivially_destructible_v<UData>) {
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~UData();
            }
        }
        ::Mlib::aligned_free(data_);
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
