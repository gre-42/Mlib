#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <vector>

namespace Mlib {

void render(
    const std::vector<ColoredVertex>& vertices,
    bool rotate = false,
    Array<float>* output = nullptr);

void render(
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const Array<float>& intrinsic_matrix,
    bool rotate = false,
    Array<float>* output = nullptr);

}
