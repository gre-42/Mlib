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

template<>
struct std::hash<Mlib::FixedArray<float, 3>>
{
    std::size_t operator()(const Mlib::FixedArray<float, 3>& s) const noexcept {
        return Mlib::fixed_array_hash(s);
    }
};

template<>
struct std::hash<Mlib::FixedArray<float, 4>>
{
    std::size_t operator()(const Mlib::FixedArray<float, 4>& s) const noexcept {
        return Mlib::fixed_array_hash(s);
    }
};
