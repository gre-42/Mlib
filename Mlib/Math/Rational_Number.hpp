#pragma once

namespace Mlib {

template <class T>
struct RationalNumber {
    T n;
    T d;
    template <class T>
    T as_float() const {
        return T(n) / T(d);
    }
};

}
