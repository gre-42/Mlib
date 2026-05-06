#include "Interior_Texture_Set.hpp"
#include <stdexcept>

using namespace Mlib;

size_t Mlib::size(InteriorTextureSet a) {
    if (!any(a)) {
        return 0;
    }
    if (!any(a & InteriorTextureSet::INTERIOR_COLORS)) {
        throw std::runtime_error("Interior textures have not colors");
    }
    for (size_t result = 5;;) {
        a >>= 1;
        if (!any(a)) {
            return result;
        }
        if (any(a & (InteriorTextureSet)1)) {
            ++result;
        }
    }
}

size_t Mlib::index(InteriorTextureSet available, InteriorTextureSet x) {
    if (!any(available & InteriorTextureSet::INTERIOR_COLORS)) {
        throw std::runtime_error("Interior colors not set");
    }
    if (!any(available & x)) {
        throw std::runtime_error("Interior texture not found");
    }
    if (!any(available & InteriorTextureSet::INTERIOR_COLORS)) {
        throw std::runtime_error("Interior textures have not colors");
    }
    available >>= 1;
    x >>= 1;
    for (size_t result = 4;;) {
        if (!any(x)) {
            if (result < 5) {
                throw std::runtime_error("Interior texture index too small");
            }
            return result;
        }
        if (any(available & (InteriorTextureSet)1)) {
            ++result;
        }
        available >>= 1;
        x >>= 1;
    }
    throw std::runtime_error("Could not find interior texture index");
}
