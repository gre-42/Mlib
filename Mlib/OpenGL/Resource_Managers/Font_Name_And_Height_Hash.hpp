#pragma once
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/OpenGL/Resource_Managers/Font_Name_And_Height.hpp>

template<>
struct std::hash<Mlib::FontNameAndHeight>
{
    std::size_t operator()(const Mlib::FontNameAndHeight& s) const noexcept {
        return s.hash.get();
    }
};
