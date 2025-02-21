#include "Font_Name_And_Height.hpp"
#include <Mlib/Hash.hpp>

using namespace Mlib;

FontNameAndHeight& FontNameAndHeight::compute_hash() {
    hash = Mlib::hash_combine(
        charset,
        ttf_filename,
        height_pixels);
    return *this;
}
