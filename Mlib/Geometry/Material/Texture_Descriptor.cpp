#include "Texture_Descriptor.hpp"

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const TextureDescriptor& t) {
    ostr <<
        "color: " << t.color << '\n' <<
        "specular: " << t.specular << '\n' <<
        "normal: " << t.normal << '\n' <<
        "color_mode: " << color_mode_to_string(t.color_mode) << '\n' <<
        "mipmap_mode: " << mipmap_mode_to_string(t.mipmap_mode) << '\n' <<
        "anisotropic_filtering_level: " << t.anisotropic_filtering_level << '\n';
    return ostr;
}
