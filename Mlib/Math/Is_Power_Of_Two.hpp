#pragma once

namespace Mlib {

template <class TData>
bool is_power_of_two(TData n) {
    // From: https://stackoverflow.com/questions/108318/whats-the-simplest-way-to-test-whether-a-number-is-a-power-of-2-in-c/108360#108360
    return n > 0 && ((n & (n - 1)) == 0);
}

}
