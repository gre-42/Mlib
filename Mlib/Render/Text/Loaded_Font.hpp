#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <stb/stb_truetype.h>
#include <vector>

namespace Mlib {

struct LoadedFont {
    GLuint texture_handle;
    float bottom_y;
    std::vector<stbtt_bakedchar> cdata;
};

}
