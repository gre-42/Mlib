#include "StbImageX.hpp"
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

std::variant<StbImage1, StbImage3, StbImage4> Mlib::load_stb_image(
    const std::string& filename)
{
    StbInfo stb_info = stb_load(filename, false, false);
    if (stb_info.nrChannels == 1) {
        return StbImage1{stb_info};
    } else if (stb_info.nrChannels == 3) {
        return StbImage3{stb_info};
    } else if (stb_info.nrChannels == 4) {
        return StbImage4{stb_info};
    } else {
        THROW_OR_ABORT("Image does not have 1, 3 or 4 channels: \"" + filename + '"');
    }
}
