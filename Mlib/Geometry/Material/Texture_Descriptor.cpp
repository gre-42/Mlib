#include "Texture_Descriptor.hpp"

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const TextureDescriptor& t) {
    ostr <<
        "color: " << t.color << '\n' <<
        "specular: " << t.specular << '\n' <<
        "normal: " << t.normal << '\n';
    return ostr;
}
