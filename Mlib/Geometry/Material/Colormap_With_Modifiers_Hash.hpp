#pragma once
#include <cstddef>

namespace Mlib {

struct ColormapWithModifiers;

}

namespace std {

template <class Key>
struct hash;

}

template <>
struct std::hash<Mlib::ColormapWithModifiers>
{
    std::size_t operator() (const Mlib::ColormapWithModifiers& k) const;
};
