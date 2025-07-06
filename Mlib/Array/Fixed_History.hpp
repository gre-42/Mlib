#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>

namespace Mlib {

template <class TData, size_t max_length>
class FixedHistory {
public:
    FixedHistory()
        : data(uninitialized)
        , next_{ 0 }
        , length_{ 0 }
    {}
    void clear() {
        static_assert(max_length > 0);
        next_ = 0;
        length_ = 0;
    }
    void append(const TData& value) {
        static_assert(max_length > 0);
        data(next_) = value;
        next_ = (next_ + 1) % max_length;
        length_ = std::min(max_length, length_ + 1);
    }
    size_t length() const {
        return length_;
    }
    bool empty() const {
        return (length_ == 0);
    }
    FixedArray<TData, max_length> data;
private:
    size_t next_;
    size_t length_;
};

template <class TData, size_t length>
TData max(const FixedHistory<TData, length>& b) {
    if (b.empty()) {
        THROW_OR_ABORT("Cannot compute maximum of empty history");
    }
    TData result = b.data(0);
    for (size_t i = 1; i < b.length(); ++i) {
        result = std::max(result, b.data(i));
    }
    return result;
}

}
