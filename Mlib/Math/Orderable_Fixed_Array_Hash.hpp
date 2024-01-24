#pragma once
#include <Mlib/Array/Fixed_Array_Hash.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <functional>

template<>
struct std::hash<Mlib::OrderableFixedArray<float, 3>>
{
    std::size_t operator()(const Mlib::OrderableFixedArray<float, 3>& s) const noexcept {
        return Mlib::fixed_array_hash(s);
    }
};

template<>
struct std::hash<Mlib::OrderableFixedArray<float, 4>>
{
    std::size_t operator()(const Mlib::OrderableFixedArray<float, 4>& s) const noexcept {
        return Mlib::fixed_array_hash(s);
    }
};
