#pragma once

namespace Mlib {

template <class TNumber>
inline void masked_set(TNumber& lhs, TNumber rhs, TNumber mask) {
    lhs = (lhs & ~mask) | (rhs & mask);
}

}
