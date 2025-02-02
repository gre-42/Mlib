#pragma once

namespace Mlib {

template <class TInt>
struct RationalNumber {
    TInt n;
    TInt d;
    template <class TFloat>
    TFloat as_float() const {
        return TFloat(n) / TFloat(d);
    }
};

}
