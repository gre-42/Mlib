#include "Imposter_Parameters.hpp"
#include <Mlib/Geometry/Cameras/Frustum_Camera_Config.hpp>

using namespace Mlib;

ImposterParameters::ImposterParameters(
    const FrustumCameraConfig& frustum,
    const FrustumCameraConfig& scaled_frustum)
{
    auto fmin = FixedArray<float, 2>{frustum.left, frustum.bottom};
    auto fmax = FixedArray<float, 2>{frustum.right, frustum.top};
    auto sfmin = FixedArray<float, 2>{scaled_frustum.left, scaled_frustum.bottom};
    auto sfmax = FixedArray<float, 2>{scaled_frustum.right, scaled_frustum.top};
    auto sfsize = sfmax - sfmin;
    pos = AxisAlignedBoundingBox<float, 2>{fmin, fmax};
    uv = AxisAlignedBoundingBox<float, 2>{
        (fmin - sfmin) / sfsize,
        (fmax - sfmin) / sfsize};
}
