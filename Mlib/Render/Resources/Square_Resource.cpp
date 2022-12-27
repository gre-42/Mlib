#include "Square_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>

using namespace Mlib;

SquareResource::SquareResource(
    const FixedArray<float, 2, 2>& square,
    const FixedArray<float, 2, 2>& uv,
    const TransformationMatrix<float, float, 3>& transformation,
    const Material& material)
{
    if (material.number_of_frames == 0) {
        throw std::runtime_error("SquareResource: material.number_of_frames is zero");
    }
    std::vector<FixedArray<ColoredVertex<float>, 3>> triangles;
    triangles.reserve(2);

    ColoredVertex<float> v00{ // min(x), min(y)
        {square(0, 0), square(0, 1), 0.f},
        fixed_ones<float, 3>(),
        {uv(0, 0), uv(0, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};
    ColoredVertex<float> v01{ // min(x), max(y)
        {square(0, 0), square(1, 1), 0.f},
        fixed_ones<float, 3>(),
        {uv(0, 0), uv(1, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};
    ColoredVertex<float> v10{ // max(x), min(y)
        {square(1, 0), square(0, 1), 0.f},
        fixed_ones<float, 3>(),
        {uv(1, 0) / (float)material.number_of_frames, uv(0, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};
    ColoredVertex<float> v11{ // max(x), max(y)
        {square(1, 0), square(1, 1), 0.f},
        fixed_ones<float, 3>(),
        {uv(1, 0) / (float)material.number_of_frames, uv(1, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};

    auto r = transformation.R() / transformation.get_scale();
    triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v00.transformed(transformation, r), v11.transformed(transformation, r), v01.transformed(transformation, r)});
    triangles.push_back(FixedArray<ColoredVertex<float>, 3>{v11.transformed(transformation, r), v00.transformed(transformation, r), v10.transformed(transformation, r)});

    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "SquareResource",
            material,
            PhysicsMaterial::ATTR_VISIBLE,
            std::move(triangles),
            std::move(std::vector<FixedArray<ColoredVertex<float>, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())));
}

void SquareResource::preload() const {
    rva_->preload();
}

void SquareResource::instantiate_renderable(const InstantiationOptions& options) const
{
    rva_->instantiate_renderable(options);
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
