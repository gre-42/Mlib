#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <vector>

namespace Mlib {

void render(
    const std::vector<ColoredVertex<float>>& vertices,
    bool rotate = false,
    Array<float>* output = nullptr);

void render_depth_map(
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    float z_offset = 0,
    bool rotate = false,
    Array<float>* output = nullptr);

void render_height_map(
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, float, 2>& normalization_matrix,
    bool rotate = false,
    Array<float>* output = nullptr);

}
