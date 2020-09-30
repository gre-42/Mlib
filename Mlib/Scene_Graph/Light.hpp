#pragma once

namespace Mlib {

struct Light {
    FixedArray<float, 3> ambience{0.5, 0.5, 0.5};
    FixedArray<float, 3> diffusivity{1, 1, 1};
    FixedArray<float, 3> specularity{1, 1, 1};
    size_t resource_index;
    bool only_black;
    bool shadow;
};

}
