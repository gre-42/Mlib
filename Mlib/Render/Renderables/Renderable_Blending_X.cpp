#include "Renderable_Blending_X.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

RenderableBlendingX::RenderableBlendingX(
    const FixedArray<float, 2, 2>& square,
    const std::string& texture,
    RenderingResources* rendering_resources)
: square_{square}
{
    for(size_t i = 0; i < 2; ++i) {
        ColoredVertex v00{ // min(x), min(y)
                {square(0, 0) / 2, square(0, 1), 0},
                fixed_ones<float, 3>(),
                {i / 2.f, 0},
                {0, 0, 1}};
        ColoredVertex v01{ // min(x), max(y)
                {square(0, 0) / 2, square(1, 1), 0},
                fixed_ones<float, 3>(),
                {i / 2.f, 1},
                {0, 0, 1}};
        ColoredVertex v10{ // max(x), min(y)
                {square(1, 0) / 2, square(0, 1), 0},
                fixed_ones<float, 3>(),
                {(1 + i) / 2.f, 0},
                {0, 0, 1}};
        ColoredVertex v11{ // max(x), max(y)
                {square(1, 0) / 2, square(1, 1), 0},
                fixed_ones<float, 3>(),
                {(1 + i) / 2.f, 1},
                {0, 0, 1}};

        std::vector<FixedArray<ColoredVertex, 3>> triangles;
        triangles.reserve(2);
        triangles.push_back(FixedArray<ColoredVertex, 3>{v00, v11, v01});
        triangles.push_back(FixedArray<ColoredVertex, 3>{v11, v00, v10});

        rva_(i) = std::make_shared<RenderableColoredVertexArray>(
            std::make_shared<ColoredVertexArray>(
                "RenderableBlendingX",
                Material{
                    texture_descriptor: {color: texture},
                    occluder_type: OccluderType::OFF,
                    blend_mode: BlendMode::CONTINUOUS,
                    clamp_mode_s: ClampMode::EDGE,
                    clamp_mode_t: ClampMode::EDGE,
                    collide: false,
                    aggregate_mode: AggregateMode::SORTED_CONTINUOUSLY,
                    is_small: true,
                    cull_faces: false,
                    ambience: {2, 2, 2},
                    diffusivity: {0, 0, 0},
                    specularity: {0, 0, 0}}.compute_color_mode(),
                std::move(triangles),
                std::move(std::vector<FixedArray<ColoredVertex, 2>>())),
            nullptr,
            rendering_resources);
    }
}

void RenderableBlendingX::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    // std::unique_lock lock_guard0{scene.dynamic_mutex_};
    // std::unique_lock lock_guard1{scene.static_mutex_};
    // std::unique_lock lock_guard2{scene.aggregate_mutex_};
    {
        auto node = new SceneNode;
        node->set_rotation({0, 0, 0});
        node->set_position({(square_(1, 0) - square_(0, 0)) / 4, 0, 0});
        rva_(1)->instantiate_renderable("plane", *node, SceneNodeResourceFilter{});
        scene_node.add_aggregate_child(name + "+0", node);
    }
    {
        auto node = new SceneNode;
        node->set_rotation({0, 0, 0});
        node->set_position({-(square_(1, 0) - square_(0, 0)) / 4, 0, 0});
        rva_(0)->instantiate_renderable("plane", *node, SceneNodeResourceFilter{});
        scene_node.add_aggregate_child(name + "-0", node);
    }
    {
        auto node = new SceneNode;
        node->set_rotation({0, -M_PI / 2, 0});
        node->set_position({0, 0, (square_(1, 1) - square_(0, 1)) / 4});
        rva_(1)->instantiate_renderable("plane", *node, SceneNodeResourceFilter{});
        scene_node.add_aggregate_child(name + "+1", node);
    }
    {
        auto node = new SceneNode;
        node->set_rotation({0, -M_PI / 2, 0});
        node->set_position({0, 0, -(square_(1, 1) - square_(0, 1)) / 4});
        rva_(0)->instantiate_renderable("plane", *node, SceneNodeResourceFilter{});
        scene_node.add_aggregate_child(name + "-1", node);
    }
}
