#pragma once
#include <cassert>
#include <cstddef>

namespace Mlib {

template <class T>
struct alignas(T) ObjectBlob {
    std::byte data[sizeof(T)];
};

/*
 * std::vector replacement to circumvent <bool> template specialization
 */
template <class T>
class NonCopyingVector {
    NonCopyingVector(const NonCopyingVector&) = delete;
    NonCopyingVector& operator = (const NonCopyingVector&) = delete;
    ObjectBlob<T>* data_;
    size_t capacity_;
    size_t size_;
public:
    inline explicit NonCopyingVector(size_t capacity)
        : data_{ new ObjectBlob<T>[capacity] }
        , capacity_{ capacity }
        , size_{ 0 }
    {}
    inline ~NonCopyingVector() {
        for (size_t i = 0; i < size(); ++i) {
            (*this)[i].~T();
        }
        delete [] data_;
    }
    template <class... Args>
    inline T& emplace_back(Args... args) {
        return *new(data_[size_++].data) T(std::forward<Args>(args)...);
    }
    inline const T& operator [] (size_t index) const {
        assert(index < size_);
        return reinterpret_cast<T&>(data_[index]);
    }
    inline T& operator [] (size_t index) {
        const NonCopyingVector& a = *this;
        return const_cast<T&>(a[index]);
    }
    inline size_t size() const {
        return size_;
    }
    inline bool empty() const {
        return size_ == 0;
    }
    T* begin() {
        return empty() ? nullptr : &(*this)[0];
    }
    T* end() {
        return empty() ? nullptr : &(*this)[size_];
    }
};

}
