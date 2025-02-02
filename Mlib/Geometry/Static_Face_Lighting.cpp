#include "Static_Face_Lighting.hpp"
#include <Mlib/Geometry/Triangle_Normal.hpp>

using namespace Mlib;

StaticFaceLighting::StaticFaceLighting(bool swap_yz)
: light_direction{0.2f, 0.8f, 0.4f},
  diffuse_color{0.6f, 0.6f, 0.6f},
  ambient_color{0.5f, 0.5f, 0.5f}
{
    light_direction /= std::sqrt(sum(squared(light_direction)));
    if (swap_yz) {
        std::swap(light_direction(1), light_direction(2));
    }
}

FixedArray<float, 3> StaticFaceLighting::get_color(
    const FixedArray<float, 3>& reflectance,
    const FixedArray<float, 3>& normal) const
{
    return reflectance * (
        ambient_color +
        diffuse_color * std::pow(std::abs(dot0d(normal, light_direction)), 2.f));
}
