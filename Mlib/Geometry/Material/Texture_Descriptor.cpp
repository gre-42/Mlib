#include "Texture_Descriptor.hpp"

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const TextureDescriptor& t) {
    ostr <<
        "color: " << t.color << '\n' <<
        "specular: " << t.specular << '\n' <<
        "normal: " << t.normal << '\n' <<
        "color/color_mode: " << color_mode_to_string(t.color.color_mode) << '\n' <<
        "color/mipmap_mode: " << mipmap_mode_to_string(t.color.mipmap_mode) << '\n' <<
        "color/anisotropic_filtering_level: " << t.color.anisotropic_filtering_level << '\n';
    return ostr;
}
