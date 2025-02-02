#pragma once
#include <Mlib/Geometry/Cameras/Perspective_Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <list>
#include <memory>
#include <vector>

namespace Mlib {

template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;
class Camera;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class Render;
template <class TPos>
class ColoredVertexArray;
enum class NormalType;

namespace Cv {

struct DepthMapPackage;

void render_point_cloud(
    Render& render,
    const Array<TransformationMatrix<float, float, 3>>& points,
    std::unique_ptr<Camera>&& camera,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig());

void render_depth_map(
    Render& render,
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    float near_plane,
    float far_plane,
    bool rotate = false,
    float scale = 1,
    float camera_z = 0,
    const SceneGraphConfig& scene_graph_config = SceneGraphConfig());

void render_depth_maps(
    Render& render,
    const std::vector<DepthMapPackage>& packages,
    const Array<TransformationMatrix<float, float, 3>>& points,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& mesh,
    const std::vector<TransformationMatrix<float, float, 3>>& beacon_locations,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const TransformationMatrix<float, float, 3>& extrinsic_matrix,
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

}

}
