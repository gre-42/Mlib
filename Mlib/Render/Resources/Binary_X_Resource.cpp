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
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BinaryXResource::BinaryXResource(
    const FixedArray<float, 2, 2>& square,
    const Material& material_0,
    const Material& material_90)
{
    std::vector<FixedArray<ColoredVertex<float>, 3>> triangles;
    triangles.reserve(2);

    ColoredVertex<float> v00{ // min(x), min(y)
        {square(0, 0), square(0, 1), 0.f},
        fixed_ones<float, 3>(),
        {0.f, 0.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex<float> v01{ // min(x), max(y)
        {square(0, 0), square(1, 1), 0.f},
        fixed_ones<float, 3>(),
        {0.f, 1.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex<float> v10{ // max(x), min(y)
        {square(1, 0), square(0, 1), 0.f},
        fixed_ones<float, 3>(),
        {1.f, 0.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex<float> v11{ // max(x), max(y)
        {square(1, 0), square(1, 1), 0.f},
        fixed_ones<float, 3>(),
        {1.f, 1.f},
        {0.f, 0.f, 1.f}};

    triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v00, v11, v01});
    triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v11, v00, v10});
    auto triangles_0 = triangles;
    auto triangles_90 = std::move(triangles);

    rva_0_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "BinaryXResource",
            material_0,
            PhysicsMaterial::ATTR_VISIBLE,
            std::move(triangles_0),
            std::move(std::vector<FixedArray<ColoredVertex<float>, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())));

    rva_90_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "BinaryXResource",
            material_90,
            PhysicsMaterial::ATTR_VISIBLE,
            std::move(triangles_90),
            std::move(std::vector<FixedArray<ColoredVertex<float>, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())));
}

BinaryXResource::~BinaryXResource()
{}

void BinaryXResource::preload() const {
    rva_0_->preload();
    rva_90_->preload();
}

void BinaryXResource::instantiate_renderable(const InstantiationOptions& options) const
{
    rva_0_->instantiate_renderable(options);

    auto node90 = std::make_unique<SceneNode>();
    node90->set_rotation({0.f, -90.f * degrees, 0.f });
    rva_90_->instantiate_renderable(
        InstantiationOptions{
            .instance_name = options.instance_name,
            .scene_node = *node90,
            .renderable_resource_filter = RenderableResourceFilter{}});
    options.scene_node.add_child(options.instance_name + "_node90", std::move(node90));
}

AggregateMode BinaryXResource::aggregate_mode() const {
    AggregateMode am_0 = rva_0_->aggregate_mode();
    AggregateMode am_90 = rva_90_->aggregate_mode();
    if (am_0 != am_90) {
        THROW_OR_ABORT("Conflicting aggregate modes");
    }
    return am_0;
}
