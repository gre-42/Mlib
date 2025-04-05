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
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BlendingXResource::BlendingXResource(
    const FixedArray<float, 2, 2>& square,
    const FixedArray<Material, 2>& materials,
    const FixedArray<Morphology, 2>& morphology)
    : rva_{ uninitialized }
    , square_ { square }
    , aggregate_modes_{
        materials(0).aggregate_mode,
        materials(1).aggregate_mode }
{
    for (size_t i = 0; i < 2; ++i) {
        float n = (float)materials(i).number_of_frames;
        ColoredVertex<float> v00{ // min(x), min(y)
                {square(0, 0) / 2, square(0, 1), 0.f},
                Colors::WHITE,
                {(float)i / 2.f / n, 0.f},
                {0.f, 0.f, 1.f}};
        ColoredVertex<float> v01{ // min(x), max(y)
                {square(0, 0) / 2, square(1, 1), 0.f},
                Colors::WHITE,
                {(float)i / 2.f / n, 1.f},
                {0.f, 0.f, 1.f}};
        ColoredVertex<float> v10{ // max(x), min(y)
                {square(1, 0) / 2, square(0, 1), 0.f},
                Colors::WHITE,
                {float(1 + i) / 2.f / n, 0.f},
                {0.f, 0.f, 1.f}};
        ColoredVertex<float> v11{ // max(x), max(y)
                {square(1, 0) / 2, square(1, 1), 0.f},
                Colors::WHITE,
                {float(1 + i) / 2.f / n, 1.f},
                {0.f, 0.f, 1.f}};

        UUVector<FixedArray<ColoredVertex<float>, 3>> triangles;
        triangles.reserve(2);
        triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v00, v11, v01});
        triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v11, v00, v10});

        rva_(i) = std::make_shared<ColoredVertexArrayResource>(
            std::make_shared<ColoredVertexArray<float>>(
                "BlendingXResource",
                materials(i),
                morphology(i) + PhysicsMaterial::ATTR_VISIBLE,
                ModifierBacklog{},
                UUVector<FixedArray<ColoredVertex<float>, 4>>(),
                std::move(triangles),
                UUVector<FixedArray<ColoredVertex<float>, 2>>(),
                UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
                UUVector<FixedArray<float, 3>>(),
                UUVector<FixedArray<uint8_t, 3>>(),
                std::vector<UUVector<FixedArray<float, 3, 2>>>(),
                std::vector<UUVector<FixedArray<float, 3>>>(),
                UUVector<FixedArray<float, 3>>(),
                UUVector<FixedArray<float, 4>>()));
    }
}

BlendingXResource::~BlendingXResource()
{}

void BlendingXResource::preload(const RenderableResourceFilter& filter) const {
    for (const auto& rva : rva_.flat_iterable()) {
        rva->preload(filter);
    }
}

void BlendingXResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    {
        auto node = make_unique_scene_node(
            FixedArray<ScenePos, 3>{(square_(1, 0) - square_(0, 0)) / 4.f, 0.f, 0.f },
            fixed_zeros<float, 3>(),
            1.f,
            options.interpolation_mode);
        rva_(1)->instantiate_child_renderable(ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = VariableAndHash<std::string>{ "plane" },
            .scene_node = node.ref(DP_LOC),
            .interpolation_mode = options.interpolation_mode,
            .renderable_resource_filter = RenderableResourceFilter{}});
        if (aggregate_modes_(1) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node->add_aggregate_child(*options.instance_name + "+0", std::move(node));
        } else if (aggregate_modes_(1) == AggregateMode::NONE) {
            options.scene_node->add_child(*options.instance_name + "+0", std::move(node));
        } else {
            THROW_OR_ABORT("Unsupported aggregate mode in blending-x-resource");
        }
    }
    {
        auto node = make_unique_scene_node(
            FixedArray<ScenePos, 3>{
                -(square_(1, 0) - square_(0, 0)) / 4.f,
                0.f,
                0.f },
            fixed_zeros<float, 3>(),
            1.f);
        rva_(0)->instantiate_child_renderable(ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = VariableAndHash<std::string>{ "plane" },
            .scene_node = node.ref(DP_LOC),
            .interpolation_mode = options.interpolation_mode,
            .renderable_resource_filter = RenderableResourceFilter{}});
        if (aggregate_modes_(0) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node->add_aggregate_child(*options.instance_name + "-0", std::move(node));
        } else if (aggregate_modes_(0) == AggregateMode::NONE) {
            options.scene_node->add_child(*options.instance_name + "-0", std::move(node));
        } else {
            THROW_OR_ABORT("Unsupported aggregate mode in blending-x-resource");
        }
    }
    {
        auto node = make_unique_scene_node(
            FixedArray<ScenePos, 3>{0.f, 0.f, (square_(1, 1) - square_(0, 1)) / 4.f },
            FixedArray<float, 3>{0.f, -90.f * degrees, 0.f },
            1.f);
        rva_(1)->instantiate_child_renderable(ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = VariableAndHash<std::string>{ "plane" },
            .scene_node = node.ref(DP_LOC),
            .interpolation_mode = options.interpolation_mode,
            .renderable_resource_filter = RenderableResourceFilter{}});
        if (aggregate_modes_(1) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node->add_aggregate_child(*options.instance_name + "+1", std::move(node));
        } else if (aggregate_modes_(1) == AggregateMode::NONE) {
            options.scene_node->add_child(*options.instance_name + "+1", std::move(node));
        } else {
            THROW_OR_ABORT("Unsupported aggregate mode in blending-x-resource");
        }
    }
    {
        auto node = make_unique_scene_node(
            FixedArray<ScenePos, 3>{
                0.f,
                0.f,
                -(square_(1, 1) - square_(0, 1)) / 4.f },
            FixedArray<float, 3>{0.f, -90.f * degrees, 0.f },
            1.f);
        rva_(0)->instantiate_child_renderable(ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = VariableAndHash<std::string>{ "plane" },
            .scene_node = node.ref(DP_LOC),
            .interpolation_mode = options.interpolation_mode,
            .renderable_resource_filter = RenderableResourceFilter{}});
        if (aggregate_modes_(0) == AggregateMode::SORTED_CONTINUOUSLY) {
            options.scene_node->add_aggregate_child(*options.instance_name + "-1", std::move(node));
        } else if (aggregate_modes_(0) == AggregateMode::NONE) {
            options.scene_node->add_child(*options.instance_name + "-1", std::move(node));
        } else {
            THROW_OR_ABORT("Unsupported aggregate mode in blending-x-resource");
        }
    }
}
