#include "Square_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

SquareResource::SquareResource(
    const FixedArray<float, 2, 2>& square,
    const TransformationMatrix<float, 3>& transformation,
    const Material& material)
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

    triangles.push_back(FixedArray<ColoredVertex, 3>{v00.transformed(transformation), v11.transformed(transformation), v01.transformed(transformation)});
    triangles.push_back(FixedArray<ColoredVertex, 3>{v11.transformed(transformation), v00.transformed(transformation), v10.transformed(transformation)});

    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray>(
            "SquareResource",
            material,
            std::move(triangles),
            std::move(std::vector<FixedArray<ColoredVertex, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())),
        nullptr);
}

void SquareResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::shared_ptr<AnimatedColoredVertexArrays> SquareResource::get_animated_arrays() const
{
    return rva_->get_animated_arrays();
}

void SquareResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

AggregateMode SquareResource::aggregate_mode() const {
    return rva_->aggregate_mode();
}
