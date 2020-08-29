#include "Static_Face_Lightning.hpp"
#include <Mlib/Geometry/Triangle_Normal.hpp>

using namespace Mlib;

StaticFaceLightning::StaticFaceLightning(bool swap_yz)
: light_direction{0.2, 0.8, 0.4},
  diffuse_color{0.6, 0.6, 0.6},
  ambient_color{0.5, 0.5, 0.5}
{
    light_direction /= std::sqrt(sum(squared(light_direction)));
    if (swap_yz) {
        std::swap(light_direction(1), light_direction(2));
    }
}

FixedArray<float, 3> StaticFaceLightning::get_color(
    const FixedArray<float, 3>& reflectance,
    const FixedArray<float, 3>& normal) const
{
    return reflectance * (
        ambient_color +
        diffuse_color * std::pow(std::abs(dot0d(normal, light_direction)), 2.f));
}
