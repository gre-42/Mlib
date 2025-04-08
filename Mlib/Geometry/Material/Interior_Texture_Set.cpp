#include "Interior_Texture_Set.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

size_t Mlib::size(InteriorTextureSet a) {
    if (!any(a)) {
        return 0;
    }
    a >>= 1;
    for (size_t result = 5; ; ++result) {
        if (!any(a)) {
            return result;
        }
        a >>= 1;
    }
}

size_t Mlib::index(InteriorTextureSet available, InteriorTextureSet x) {
    if (!any(available & InteriorTextureSet::INTERIOR_COLORS)) {
        THROW_OR_ABORT("Interior colors not set");
    }
    for (size_t result = 5; any(available);) {
        if (any(available & x)) {
            return result;
        }
        if (any(available & (InteriorTextureSet)1)) {
            ++result;
        }
        available >>= 1;
    }
    THROW_OR_ABORT("Could not find interior texture index");
}
