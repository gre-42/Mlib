#pragma once

namespace Mlib {

// From: https://stackoverflow.com/questions/45504920/doing-saturation-aritmetic-with-atomics
template <class T>
bool bounded_atomic_dec_equals_zero(T& ctr) {
    auto ctr_old = ctr.load();
    if (ctr_old == 0) {
        return true;
    }
    ctr.compare_exchange_weak(ctr_old, ctr_old - 1);
    return ctr_old == 1;
}

}
