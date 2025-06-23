#include "Binary_X_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

BinaryXResource::BinaryXResource(
    const FixedArray<float, 2, 2>& square,
    const Material& material_0,
    const Material& material_90,
    const Morphology& morphology_0,
    const Morphology& morphology_90)
{
    UUVector<FixedArray<ColoredVertex<float>, 3>> triangles;
    triangles.reserve(2);

    ColoredVertex<float> v00{ // min(x), min(y)
        {square(0, 0), square(0, 1), 0.f},
        Colors::WHITE,
        {0.f, 0.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex<float> v01{ // min(x), max(y)
        {square(0, 0), square(1, 1), 0.f},
        Colors::WHITE,
        {0.f, 1.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex<float> v10{ // max(x), min(y)
        {square(1, 0), square(0, 1), 0.f},
        Colors::WHITE,
        {1.f, 0.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex<float> v11{ // max(x), max(y)
        {square(1, 0), square(1, 1), 0.f},
        Colors::WHITE,
        {1.f, 1.f},
        {0.f, 0.f, 1.f}};

    triangles.emplace_back(v00, v11, v01);
    triangles.emplace_back(v11, v00, v10);
    auto triangles_0 = triangles;
    auto triangles_90 = std::move(triangles);

    rva_0_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "BinaryXResource",
            material_0,
            morphology_0 + PhysicsMaterial::ATTR_VISIBLE,
            ModifierBacklog{},
            UUVector<FixedArray<ColoredVertex<float>, 4>>(),
            std::move(triangles_0),
            UUVector<FixedArray<ColoredVertex<float>, 2>>(),
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<uint8_t, 3>>(),
            std::vector<UUVector<FixedArray<float, 3, 2>>>(),
            std::vector<UUVector<FixedArray<float, 3>>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<float, 4>>()));

    rva_90_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "BinaryXResource",
            material_90,
            morphology_90 + PhysicsMaterial::ATTR_VISIBLE,
            ModifierBacklog{},
            UUVector<FixedArray<ColoredVertex<float>, 4>>(),
            std::move(triangles_90),
            UUVector<FixedArray<ColoredVertex<float>, 2>>(),
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<uint8_t, 3>>(),
            std::vector<UUVector<FixedArray<float, 3, 2>>>(),
            std::vector<UUVector<FixedArray<float, 3>>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<float, 4>>()));
}

BinaryXResource::~BinaryXResource()
{}

void BinaryXResource::preload(const RenderableResourceFilter& filter) const {
    rva_0_->preload(filter);
    rva_90_->preload(filter);
}

void BinaryXResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    rva_0_->instantiate_child_renderable(options);

    auto node90 = make_unique_scene_node(
        fixed_zeros<ScenePos, 3>(),
        FixedArray<float, 3>{0.f, -90.f * degrees, 0.f },
        1.f,
        options.interpolation_mode);
    rva_90_->instantiate_child_renderable(
        ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = options.instance_name,
            .scene_node = node90.ref(DP_LOC),
            .interpolation_mode = options.interpolation_mode,
            .renderable_resource_filter = options.renderable_resource_filter});
    options.scene_node->add_child(
        VariableAndHash<std::string>{*options.instance_name + "_node90"},
        std::move(node90));
}

std::shared_ptr<AnimatedColoredVertexArrays> BinaryXResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    auto acvas = std::make_shared<AnimatedColoredVertexArrays>();
    acvas->insert(*rva_0_->get_arrays(filter));
    acvas->insert(*rva_90_->get_arrays(filter));
    return acvas;
}

AggregateMode BinaryXResource::get_aggregate_mode() const {
    AggregateMode am_0 = rva_0_->get_aggregate_mode();
    AggregateMode am_90 = rva_90_->get_aggregate_mode();
    if (am_0 != am_90) {
        THROW_OR_ABORT("Conflicting aggregate modes");
    }
    return am_0;
}
