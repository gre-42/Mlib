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
    const Material& material_0,
    const Material& material_90)
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
        {1.f, 0.f},
        {0.f, 0.f, 1.f}};
    ColoredVertex v11{ // max(x), max(y)
        {square(1, 0), square(1, 1), 0.f},
        fixed_ones<float, 3>(),
        {1.f, 1.f},
        {0.f, 0.f, 1.f}};

    triangles.push_back(FixedArray<ColoredVertex, 3>{v00, v11, v01});
    triangles.push_back(FixedArray<ColoredVertex, 3>{v11, v00, v10});
    auto triangles_0 = triangles;
    auto triangles_90 = std::move(triangles);

    rva_0_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray>(
            "BinaryXResource",
            material_0,
            std::move(triangles_0),
            std::move(std::vector<FixedArray<ColoredVertex, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())),
        nullptr);  // instances

    rva_90_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray>(
            "BinaryXResource",
            material_90,
            std::move(triangles_90),
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

    rva_0_->instantiate_renderable(name, scene_node, SceneNodeResourceFilter());

    auto node90 = new SceneNode;
    node90->set_rotation({0.f, -float{M_PI} / 2.f, 0.f });
    rva_90_->instantiate_renderable(name, *node90, SceneNodeResourceFilter());
    scene_node.add_child(name + "_node90", node90);
}

AggregateMode BinaryXResource::aggregate_mode() const {
    AggregateMode am_0 = rva_0_->aggregate_mode();
    AggregateMode am_90 = rva_90_->aggregate_mode();
    if (am_0 != am_90) {
        throw std::runtime_error("Conflicting aggregate modes");
    }
    return am_0;
}
