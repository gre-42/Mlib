#pragma once
#include <Mlib/Geometry/Cameras/Perspective_Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

namespace Mlib {

template <class TData>
class Array;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class Render;
enum class NormalType;

#ifndef __ANDROID__
void render_height_map(
    Render& render,
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, float, 2>& normalization_matrix,
    NormalType normal_type,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
    const PerspectiveCameraConfig& camera_config = PerspectiveCameraConfig());
#endif

}
