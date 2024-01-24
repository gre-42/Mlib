#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Hash.hpp>

namespace Mlib {

template <class TData, size_t tlength>
std::size_t fixed_array_hash(const FixedArray<TData, tlength>& s) noexcept
{
    Hasher hasher;
    for (const auto& v : s.flat_iterable()) {
        hasher.combine(v);
    }
    return hasher;
}

}
