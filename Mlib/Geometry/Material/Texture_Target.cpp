#include "Texture_Target.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <ostream>
#include <string>

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, TextureTarget target) {
    switch (target) {
    case TextureTarget::NONE:
        return ostr << "none";
    case TextureTarget::TEXTURE_2D:
        return ostr << "texture_2d";
    case TextureTarget::TEXTURE_2D_ARRAY:
        return ostr << "texture_2d_array";
    case TextureTarget::TEXTURE_3D:
        return ostr << "texture_3d";
    case TextureTarget::TEXTURE_CUBE_MAP:
        return ostr << "texture_cube_map";
    case TextureTarget::ONE_LAYER_MASK:
        return ostr << "one_layer_mask";
    }
    THROW_OR_ABORT("Unknown texture type: " + std::to_string((int)target));
}
