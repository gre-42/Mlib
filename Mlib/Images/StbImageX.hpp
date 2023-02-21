#pragma once
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <variant>

namespace Mlib {

std::variant<StbImage1, StbImage3, StbImage4> load_stb_image(const std::string& filename);

}
