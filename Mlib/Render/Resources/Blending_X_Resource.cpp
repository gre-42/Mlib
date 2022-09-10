#include "Blending_X_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>

using namespace Mlib;

BlendingXResource::BlendingXResource(
    const FixedArray<float, 2, 2>& square,
    const FixedArray<Material, 2>& materials)
: square_{square},
  aggregate_modes_{
    materials(0).aggregate_mode,
    materials(1).aggregate_mode}
{
    for (size_t i = 0; i < 2; ++i) {
        float n = (float)materials(i).number_of_frames;
        ColoredVertex<float> v00{ // min(x), min(y)
                .position = {square(0, 0) / 2, square(0, 1), 0.f},
                .color = fixed_ones<float, 3>(),
                .uv = {i / 2.f / n, 0.f},
                .normal = {0.f, 0.f, 1.f}};
        ColoredVertex<float> v01{ // min(x), max(y)
                .position = {square(0, 0) / 2, square(1, 1), 0.f},
                .color = fixed_ones<float, 3>(),
                .uv = {i / 2.f / n, 1.f},
                .normal = {0.f, 0.f, 1.f}};
        ColoredVertex<float> v10{ // max(x), min(y)
                .position = {square(1, 0) / 2, square(0, 1), 0.f},
                .color = fixed_ones<float, 3>(),
                .uv = {(1 + i) / 2.f / n, 0.f},
                .normal = {0.f, 0.f, 1.f}};
        ColoredVertex<float> v11{ // max(x), max(y)
                .position = {square(1, 0) / 2, square(1, 1), 0.f},
                .color = fixed_ones<float, 3>(),
                .uv = {(1 + i) / 2.f / n, 1.f},
                .normal = {0.f, 0.f, 1.f}};

        std::vector<FixedArray<ColoredVertex<float>, 3>> triangles;
        triangles.reserve(2);
        triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v00, v11, v01});
        triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v11, v00, v10});

        rva_(i) = std::make_shared<ColoredVertexArrayResource>(
            std::make_shared<ColoredVertexArray<float>>(
                "BlendingXResource",
                materials(i),
                PhysicsMaterial::ATTR_VISIBLE,
                std::move(triangles),
                std::move(std::vector<FixedArray<ColoredVertex<float>, 2>>()),
                std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
                std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())));
    }
}

BlendingXResource::~BlendingXResource()
{}

void BlendingXResource::preload() const {
    for (const auto& rva : rva_.flat_iterable()) {
        rva->preload();
    }
}

void BlendingXResource::instantiate_renderable(const InstantiationOptions& options) const
{
    {
        auto node = std::make_unique<SceneNode>();
        node->set_rotation({0.f, 0.f, 0.f });
        node->set_position({(square_(1, 0) - square_(0, 0)) / 4.f, 0.f, 0.f });
        rva_(1)->instantiate_renderable(InstantiationOptions{
            .instance_name = "plane",
            .scene_node = *node,
            .renderable_resource_filter = RenderableResourceFilter()});
        if (aggregate_modes_(1) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node.add_aggregate_child(options.instance_name + "+0", std::move(node));
        } else if (aggregate_modes_(1) == AggregateMode::NONE) {
            options.scene_node.add_child(options.instance_name + "+0", std::move(node));
        } else {
            throw std::runtime_error("Unsupported aggregate mode in blending-x-resource");
        }
    }
    {
        auto node = std::make_unique<SceneNode>();
        node->set_rotation({0.f, 0.f, 0.f });
        node->set_position({-(square_(1, 0) - square_(0, 0)) / 4.f, 0.f, 0.f });
        rva_(0)->instantiate_renderable(InstantiationOptions{
            .instance_name = "plane",
            .scene_node = *node,
            .renderable_resource_filter = RenderableResourceFilter()});
        if (aggregate_modes_(0) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node.add_aggregate_child(options.instance_name + "-0", std::move(node));
        } else if (aggregate_modes_(0) == AggregateMode::NONE) {
            options.scene_node.add_child(options.instance_name + "-0", std::move(node));
        } else {
            throw std::runtime_error("Unsupported aggregate mode in blending-x-resource");
        }
    }
    {
        auto node = std::make_unique<SceneNode>();
        node->set_rotation({0.f, -90.f * degrees, 0.f });
        node->set_position({0.f, 0.f, (square_(1, 1) - square_(0, 1)) / 4.f });
        rva_(1)->instantiate_renderable(InstantiationOptions{
            .instance_name = "plane",
            .scene_node = *node,
            .renderable_resource_filter = RenderableResourceFilter()});
        if (aggregate_modes_(1) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node.add_aggregate_child(options.instance_name + "+1", std::move(node));
        } else if (aggregate_modes_(1) == AggregateMode::NONE) {
            options.scene_node.add_child(options.instance_name + "+1", std::move(node));
        } else {
            throw std::runtime_error("Unsupported aggregate mode in blending-x-resource");
        }
    }
    {
        auto node = std::make_unique<SceneNode>();
        node->set_rotation({0.f, -90.f * degrees, 0.f });
        node->set_position({0.f, 0.f, -(square_(1, 1) - square_(0, 1)) / 4.f });
        rva_(0)->instantiate_renderable(InstantiationOptions{
            .instance_name = "plane",
            .scene_node = *node,
            .renderable_resource_filter = RenderableResourceFilter()});
        if (aggregate_modes_(0) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node.add_aggregate_child(options.instance_name + "-1", std::move(node));
        } else if (aggregate_modes_(0) == AggregateMode::NONE) {
            options.scene_node.add_child(options.instance_name + "-1", std::move(node));
        } else {
            throw std::runtime_error("Unsupported aggregate mode in blending-x-resource");
        }
    }
}
