#include "Binary_X_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

BinaryXResource::BinaryXResource(
    const FixedArray<float, 2, 2>& square,
    const std::string& texture,
    bool is_small,
    OccluderType occluder_type,
    const FixedArray<float, 3>& ambience)
{
    std::vector<FixedArray<ColoredVertex, 3>> triangles;
    triangles.reserve(2);

    ColoredVertex v00{ // min(x), min(y)
        {square(0, 0), square(0, 1), 0.f},
        fixed_ones<float, 3>(),
        {0.f, 0.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex v01{ // min(x), max(y)
        {square(0, 0), square(1, 1), 0.f},
        fixed_ones<float, 3>(),
        {0.f, 1.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex v10{ // max(x), min(y)
        {square(1, 0), square(0, 1), 0.f},
        fixed_ones<float, 3>(),
        {1.f,0.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex v11{ // max(x), max(y)
        {square(1, 0), square(1, 1), 0.f},
        fixed_ones<float, 3>(),
        {1.f, 1.f},
        {0.f, 0.f, 1.f}};

    triangles.push_back(FixedArray<ColoredVertex, 3>{v00, v11, v01});
    triangles.push_back(FixedArray<ColoredVertex, 3>{v11, v00, v10});

    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray>(
            "BinaryXResource",
            Material{
                .blend_mode = BlendMode::BINARY,
                .textures = {{.texture_descriptor = {.color = texture}}},
                .occluder_type = occluder_type,
                .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
                .wrap_mode_t = WrapMode::CLAMP_TO_EDGE,
                .collide = false,
                .aggregate_mode = is_small ? AggregateMode::SORTED_CONTINUOUSLY : AggregateMode::ONCE,
                .is_small = is_small,
                .cull_faces = false,
                .ambience = OrderableFixedArray{ambience},
                .diffusivity = {0.f, 0.f, 0.f},
                .specularity = {0.f, 0.f, 0.f}}.compute_color_mode(),
            std::move(triangles),
            std::move(std::vector<FixedArray<ColoredVertex, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())),
        nullptr);  // instances
}

void BinaryXResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    // std::unique_lock lock_guard0{scene.dynamic_mutex_};
    // std::unique_lock lock_guard1{scene.static_mutex_};
    // std::unique_lock lock_guard2{scene.aggregate_mutex_};

    rva_->instantiate_renderable("plane", scene_node, SceneNodeResourceFilter());

    auto node90 = new SceneNode;
    node90->set_rotation({0.f, -float{M_PI} / 2.f, 0.f });
    rva_->instantiate_renderable("plane", *node90, SceneNodeResourceFilter());
    scene_node.add_aggregate_child("node90", node90);
}

AggregateMode BinaryXResource::aggregate_mode() const {
    return rva_->aggregate_mode();
}
