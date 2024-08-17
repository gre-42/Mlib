#pragma once
#include <Mlib/Default_Uninitialized.hpp>
#include <vector>

namespace Mlib {

template <class T>
using UVector = std::vector<default_uninitialized_t<T>>;

template <class T>
using UUVector = std::vector<DefaultUnitialized<T>>;

template <class TIterable>
inline auto uuvector(const TIterable& begin, const TIterable& end) {
    using T = typename TIterable::value_type;
    return UUVector<T>(begin, end);
}

}
