#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>

namespace Mlib {

template <class T>
class Span {
public:
    Span(T* begin, size_t count)
        : begin_(begin)
        , count_{ count }
    {}
    Span(T* begin, T* end)
        : begin_(begin)
        , count_{ integral_cast<size_t>(end - begin) }
    {}
    T* begin() const {
        return begin_;
    }
    T* end() const {
        return begin_ + count_;
    }
private:
    T* begin_;
    size_t count_;
};

}