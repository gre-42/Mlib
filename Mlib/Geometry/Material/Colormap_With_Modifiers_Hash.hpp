#pragma once
#include <Mlib/Std_Hash.hpp>
#include <cstddef>

namespace Mlib {

struct ColormapWithModifiers;

}

template <>
struct std::hash<Mlib::ColormapWithModifiers>
{
    std::size_t operator() (const Mlib::ColormapWithModifiers& k) const;
};
