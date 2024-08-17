#pragma once

namespace Mlib {

namespace Dff {

struct RasterConfig {
    bool need_to_read_back_textures;
    bool make_native;
    bool flip_gl_y_axis;
};

}

}
