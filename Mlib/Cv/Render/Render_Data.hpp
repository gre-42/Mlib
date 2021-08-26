#pragma once
#include <Mlib/Scene_Graph/Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;
class Camera;
template <class TData, size_t n>
class TransformationMatrix;
class Render2;

namespace Cv {

struct DepthMapPackage;

void render_point_cloud(
    Render2& render,
    const Array<FixedArray<float, 3>>& points,
    std::unique_ptr<Camera>&& camera,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig());

void render_depth_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    float near_plane,
    float far_plane,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig());

void render_depth_maps(
    Render2& render,
    const std::vector<DepthMapPackage>& packages,
    const Array<FixedArray<float, 3>>& points,
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    const TransformationMatrix<float, 3>& extrinsic_matrix,
    float width,
    float height,
    float near_plane,
    float far_plane,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
    float point_radius = 0.1f,
    float cos_threshold = 0.f);

void render_height_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, 2>& normalization_matrix,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig(),
    const CameraConfig& camera_config = CameraConfig());

}

}
