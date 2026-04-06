#pragma once
#include <Mlib/Math/Is_Power_Of_Two.hpp>
#include <bit>
#include <stdexcept>

namespace Mlib {

template <class T>
class PowerOfTwoDivider {
public:
    explicit PowerOfTwoDivider(T n)
        : n_{ n }
    {
        if (!is_power_of_two(n)) {
            throw std::runtime_error("PowerOfTwoDivider: Number is not a power of 2");
        }
    }
    T greatest_divider(const T n2) const {
        auto res = std::max<T>(1, std::bit_ceil(n2));
        if (res > n_) {
            throw std::runtime_error("PowerOfTwoDivider: Number is too large");
        }
        return res;
    }
private:
    T n_;
};

}
