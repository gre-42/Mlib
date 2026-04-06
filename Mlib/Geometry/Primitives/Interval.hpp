#pragma once
#include <cstdint>

namespace Mlib {

template <class T>
struct Interval {
    T min;
    T max;
    T center() const {
        return (min + max) / (uint16_t)2;
    }
    T length() const {
        return max - min;
    }
};

template <class T>
Interval<T> operator * (const Interval<T>& a, const T& b) {
    return { a.min * b, a.max * b };
}

}
