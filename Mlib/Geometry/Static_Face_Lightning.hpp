#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class StaticFaceLightning {
public:
    explicit StaticFaceLightning(bool swap_yz = false);
    FixedArray<float, 3> get_color(
        const FixedArray<float, 3>& reflectance,
        const FixedArray<float, 3>& normal) const;
private:
    FixedArray<float, 3> light_direction;
    FixedArray<float, 3> diffuse_color;
    FixedArray<float, 3> ambient_color;
};

}
