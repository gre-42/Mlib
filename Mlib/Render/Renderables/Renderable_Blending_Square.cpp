#include "Renderable_Blending_Square.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

RenderableBlendingSquare::RenderableBlendingSquare(
    const FixedArray<float, 2, 2>& square,
    const std::string& texture,
    RenderingResources* rendering_resources)
{
    std::vector<FixedArray<ColoredVertex, 3>> triangles;
    triangles.reserve(2);

    ColoredVertex v00{ // min(x), min(y)
            {square(0, 0), square(0, 1), 0},
            fixed_ones<float, 3>(),
            {0, 0},
            {0, 0, 1}};
    ColoredVertex v01{ // min(x), max(y)
            {square(0, 0), square(1, 1), 0},
            fixed_ones<float, 3>(),
            {0, 1},
            {0, 0, 1}};
    ColoredVertex v10{ // max(x), min(y)
            {square(1, 0), square(0, 1), 0},
            fixed_ones<float, 3>(),
            {1, 0},
            {0, 0, 1}};
    ColoredVertex v11{ // max(x), max(y)
            {square(1, 0), square(1, 1), 0},
            fixed_ones<float, 3>(),
            {1, 1},
            {0, 0, 1}};

    triangles.push_back(FixedArray<ColoredVertex, 3>{v00, v11, v01});
    triangles.push_back(FixedArray<ColoredVertex, 3>{v11, v00, v10});

    rva_ = std::make_shared<RenderableColoredVertexArray>(
        std::make_shared<ColoredVertexArray>(
                "",
                Material{
                    texture: texture,
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
                    specularity: {0, 0, 0}},
                std::move(triangles),
                std::move(std::vector<FixedArray<ColoredVertex, 2>>())),
        nullptr,
        rendering_resources);
}

void RenderableBlendingSquare::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter)
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableBlendingSquare::get_triangle_meshes()
{
    return rva_->get_triangle_meshes();
}

void RenderableBlendingSquare::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
