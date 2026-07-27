#pragma once
#include <concepts>
#include <optional>
#include <ostream>

namespace Mlib {

template <std::integral T>
std::ostream& operator << (std::ostream& ostr, const std::optional<T>& opt) {
    if (opt.has_value()) {
        ostr << (*opt + 0);
    } else {
        ostr << "none";
    }
    return ostr;
}

}
