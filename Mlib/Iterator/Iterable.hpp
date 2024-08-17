#pragma once
#include <utility>

namespace Mlib {

template <class T>
class Iterable {
public:
    Iterable(T begin, T end)
        : begin_{ std::move(begin) }
        , end_{ std::move(end) }
    {}
    const T& begin() {
        return begin_;
    }
    const T& end() {
        return end_;
    }
private:
    T begin_;
    T end_;
};

}