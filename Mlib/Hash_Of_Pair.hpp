#pragma once
#include <Mlib/Hash.hpp>
#include <utility>

template <class T1, class T2>
struct std::hash<std::pair<T1, T2>>
{
    std::size_t operator() (const std::pair<T1, T2>& a) const {
        return Mlib::hash_combine(a.first, a.second);
    }
};
