#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

/**
 * From: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
 */
template <class TData>
FixedArray<TData, 3> triangle_tangent(
    const FixedArray<TData, 3>& pos0,
    const FixedArray<TData, 3>& pos1,
    const FixedArray<TData, 3>& pos2,
    const FixedArray<TData, 2>& uv0,
    const FixedArray<TData, 2>& uv1,
    const FixedArray<TData, 2>& uv2)
{
    FixedArray<TData, 3> edge0 = pos1 - pos0;
    FixedArray<TData, 3> edge1 = pos2 - pos0;
    FixedArray<TData, 2> deltaUV0 = uv1 - uv0;
    FixedArray<TData, 2> deltaUV1 = uv2 - uv0;

    TData f = 1.0f / (deltaUV0(0) * deltaUV1(1) - deltaUV1(0) * deltaUV0(1));

    FixedArray<TData, 3> tangent1 = f * (deltaUV1(1) * edge0 - deltaUV0(1) * edge1);
    tangent1 /= std::sqrt(sum(squared(tangent1)));
    return tangent1;
}

}
