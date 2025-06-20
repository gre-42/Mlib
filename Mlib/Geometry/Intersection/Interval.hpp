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

}
