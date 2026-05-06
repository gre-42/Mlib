#include "Colormap_With_Modifiers_Hash.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Hashing/Hash.hpp>

using namespace Mlib;

std::size_t std::hash<Mlib::ColormapWithModifiers>::operator() (const Mlib::ColormapWithModifiers& k) const {
    return k.hash.get();
}
