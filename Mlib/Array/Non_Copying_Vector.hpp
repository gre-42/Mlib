#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
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
        clear();
        delete [] data_;
    }
    void clear() {
        while (size_ > 0) {
            (*this)[size_ - 1].~T();
            --size_;
        }
    }
    void clear_and_reserve(size_t capacity) {
        clear();
        delete [] data_;
        data_ = new ObjectBlob<T>[capacity];
        capacity_ = capacity;
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
    inline const T& at(size_t index) const {
        if (index >= size_) {
            THROW_OR_ABORT("Index out of bounds");
        }
        return (*this)[index];
    }
    inline T& at(size_t index) {
        const auto& a = *this;
        return const_cast<T&>(a.at(index));
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
