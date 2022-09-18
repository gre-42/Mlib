#pragma once
#include <Mlib/Geometry/Cameras/Perspective_Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

namespace Mlib {

template <class TData>
class Array;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class Render2;
enum class NormalType;

void render_height_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, float, 2>& normalization_matrix,
    NormalType normal_type,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
    const PerspectiveCameraConfig& camera_config = PerspectiveCameraConfig());

}
