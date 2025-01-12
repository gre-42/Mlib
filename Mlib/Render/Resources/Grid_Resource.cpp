#include "Grid_Resource.hpp"
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Terrain_Uv.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

GridResource::GridResource(
    const FixedArray<size_t, 2>& size,
    const TransformationMatrix<float, double, 3>& transformation,
    double tile_length,
    double scale,
    double uv_scale,
    double period,
    const Material& material,
    const Morphology& morphology)
{
    auto trafo = transformation;
    trafo.t *= scale;
    TriangleList<CompressedScenePos> triangles{
        "grid",
        material,
        morphology + (PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE) };

    for (size_t r = 0; r < size(0); ++r) {
        for (size_t c = 0; c < size(1); ++c) {
            auto p00 = scale * tile_length * FixedArray<double, 2>{(double)r, (double)c};
            auto p10 = scale * tile_length * FixedArray<double, 2>{(double)r + 1, (double)c};
            auto p11 = scale * tile_length * FixedArray<double, 2>{(double)r + 1, (double)c + 1};
            auto p01 = scale * tile_length * FixedArray<double, 2>{(double)r, (double)c + 1};
            auto uv = terrain_uv(p00, p10, p11, p01, scale, uv_scale, period);
            triangles.draw_rectangle_wo_normals(
                trafo.transform(FixedArray<double, 3>{p00(0), p00(1), 0.}).casted<CompressedScenePos>(),
                trafo.transform(FixedArray<double, 3>{p10(0), p10(1), 0.}).casted<CompressedScenePos>(),
                trafo.transform(FixedArray<double, 3>{p11(0), p11(1), 0.}).casted<CompressedScenePos>(),
                trafo.transform(FixedArray<double, 3>{p01(0), p01(1), 0.}).casted<CompressedScenePos>(),
                Colors::WHITE,
                Colors::WHITE,
                Colors::WHITE,
                Colors::WHITE,
                uv[0],
                uv[1],
                uv[2],
                uv[3]);
        }
    }

    rva_ = std::make_shared<ColoredVertexArrayResource>(triangles.triangle_array());
}

void GridResource::preload(const RenderableResourceFilter& filter) const
{
    rva_->preload(filter);
}

void GridResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    rva_->instantiate_child_renderable(options);
}

void GridResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{
    rva_->instantiate_root_renderables(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> GridResource::get_physics_arrays() const
{
    return rva_->get_physics_arrays();
}

void GridResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

AggregateMode GridResource::get_aggregate_mode() const {
    return rva_->get_aggregate_mode();
}

std::list<SpawnPoint> GridResource::get_spawn_points() const
{
    return {};
}

WayPointSandboxes GridResource::get_way_points() const
{
    return {};
}

void GridResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    return rva_->modify_physics_material_tags(add, remove, filter);
}

void GridResource::generate_instances() {
    // Do nothing
}

void GridResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    // Do nothing
}
