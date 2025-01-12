#include "Square_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SquareResource::SquareResource(
    const FixedArray<float, 2, 2>& square,
    const FixedArray<float, 2, 2>& uv,
    const TransformationMatrix<float, float, 3>& transformation,
    const Material& material,
    const Morphology& morphology)
{
    if (material.number_of_frames == 0) {
        THROW_OR_ABORT("SquareResource: material.number_of_frames is zero");
    }

    ColoredVertex<float> v00{ // min(x), min(y)
        {square(0, 0), square(0, 1), 0.f},
        Colors::WHITE,
        {uv(0, 0), uv(0, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};
    ColoredVertex<float> v01{ // min(x), max(y)
        {square(0, 0), square(1, 1), 0.f},
        Colors::WHITE,
        {uv(0, 0), uv(1, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};
    ColoredVertex<float> v10{ // max(x), min(y)
        {square(1, 0), square(0, 1), 0.f},
        Colors::WHITE,
        {uv(1, 0) / (float)material.number_of_frames, uv(0, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};
    ColoredVertex<float> v11{ // max(x), max(y)
        {square(1, 0), square(1, 1), 0.f},
        Colors::WHITE,
        {uv(1, 0) / (float)material.number_of_frames, uv(1, 1)},
        {0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f}};

    auto r = transformation.R / transformation.get_scale();
    UUVector<FixedArray<ColoredVertex<float>, 3>> triangles{
        FixedArray<ColoredVertex<float>, 3>{v00.transformed(transformation, r), v11.transformed(transformation, r), v01.transformed(transformation, r)},
        FixedArray<ColoredVertex<float>, 3>{v11.transformed(transformation, r), v00.transformed(transformation, r), v10.transformed(transformation, r)}
    };

    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "SquareResource_" + material.identifier(),
            material,
            morphology + PhysicsMaterial::ATTR_VISIBLE,
            ModifierBacklog{},
            UUVector<FixedArray<ColoredVertex<float>, 4>>(),
            std::move(triangles),
            UUVector<FixedArray<ColoredVertex<float>, 2>>(),
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<uint8_t, 3>>(),
            std::vector<UUVector<FixedArray<float, 3, 2>>>(),
            std::vector<UUVector<FixedArray<float, 3>>>(),
            UUVector<FixedArray<float, 3>>()));
}

void SquareResource::preload(const RenderableResourceFilter& filter) const {
    rva_->preload(filter);
}

void SquareResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    rva_->instantiate_child_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> SquareResource::get_physics_arrays() const
{
    return rva_->get_physics_arrays();
}

void SquareResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

AggregateMode SquareResource::get_aggregate_mode() const {
    return rva_->get_aggregate_mode();
}
