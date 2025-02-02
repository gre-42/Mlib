#pragma once
#include <Mlib/Default_Uninitialized.hpp>
#include <list>

namespace Mlib {

template <class T>
using UList = std::list<default_uninitialized_t<T>>;

template <class T>
using UUList = std::list<DefaultUnitialized<T>>;

template <class TIterable>
inline auto uulist(const TIterable& begin, const TIterable& end) {
    using T = typename TIterable::value_type;
    return UUList<T>(begin, end);
}

}
