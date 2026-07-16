#pragma once
#include <Mlib/Initialization/Default_Uninitialized.hpp>
#include <Mlib/Os/Os.hpp>
#include <cassert>
#include <cstdlib>
#include <sstream>

namespace Mlib {

void debug_vector_allocation(bool value);
bool debug_vector_allocation();

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
    std::align_val_t alignment_;
public:
    inline Vector(size_t size, size_t alignment, Uninitialized)
        : size_{size}
        , alignment_{(std::align_val_t)alignment}
    {
        if (debug_vector_allocation()) {
            linfo() << "Allocate vector. Size: " << size << ", alignment: " << alignment;
        }
        if (alignment % alignof(UData) != 0) {
            throw std::runtime_error((std::stringstream() << "Alignment is not a multiple of 'alignof'. " <<
                "Alignment (must be a power of 2): " << alignment <<
                ", alignof: " << alignof(UData)).str());
        }
        data_ = (UData*) ::operator new[](size * sizeof(UData), alignment_);
        if constexpr (!std::is_trivially_constructible_v<UData>) {
            struct UnwindGuard {
                UData* ptr;
                size_t constructed_count;
                std::align_val_t alignment;
                inline ~UnwindGuard() {
                    if (ptr != nullptr) {
                        if constexpr (!std::is_trivially_destructible_v<UData>) {
                            for (size_t i = constructed_count; i > 0; --i) {
                                ptr[i - 1].~UData();
                            }
                        }
                        ::operator delete[](ptr, alignment);
                    }
                }
            } guard{ data_, 0, alignment_ };
            for (; guard.constructed_count < size_; ++guard.constructed_count) {
                ::new (data_ + guard.constructed_count) UData();
            }
            guard.ptr = nullptr;
        }
    }
    inline ~Vector() {
        if (debug_vector_allocation()) {
            linfo() << "vector dtor";
        }
        if constexpr (!std::is_trivially_destructible_v<UData>) {
            if (debug_vector_allocation()) {
                linfo() << "call dtors";
            }
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~UData();
            }
        }
        ::operator delete[](data_, alignment_);
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
