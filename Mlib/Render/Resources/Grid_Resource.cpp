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
    double scale,
    double uv_scale,
    double period,
    const Material& material)
{
    TriangleList<double> triangles{
        "grid",
        material,
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE};

    for (size_t r = 0; r < size(0); ++r) {
        for (size_t c = 0; c < size(1); ++c) {
            auto p00 = scale * FixedArray<double, 2>{(double)r, (double)c};
            auto p10 = scale * FixedArray<double, 2>{(double)r + 1, (double)c};
            auto p11 = scale * FixedArray<double, 2>{(double)r + 1, (double)c + 1};
            auto p01 = scale * FixedArray<double, 2>{(double)r, (double)c + 1};
            auto uv = terrain_uv(p00, p10, p11, p01, 1.0, uv_scale, period);
            triangles.draw_rectangle_wo_normals(
                transformation.transform(FixedArray<double, 3>{p00(0), p00(1), 0.}),
                transformation.transform(FixedArray<double, 3>{p10(0), p10(1), 0.}),
                transformation.transform(FixedArray<double, 3>{p11(0), p11(1), 0.}),
                transformation.transform(FixedArray<double, 3>{p01(0), p01(1), 0.}),
                fixed_ones<float, 3>(),
                fixed_ones<float, 3>(),
                fixed_ones<float, 3>(),
                fixed_ones<float, 3>(),
                uv(0),
                uv(1),
                uv(2),
                uv(3));
        }
    }

    rva_ = std::make_shared<ColoredVertexArrayResource>(triangles.triangle_array());
}

void GridResource::preload() const
{
    rva_->preload();
}

void GridResource::instantiate_renderable(const InstantiationOptions& options) const
{
    rva_->instantiate_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> GridResource::get_physics_arrays() const
{
    return rva_->get_physics_arrays();
}

void GridResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

AggregateMode GridResource::aggregate_mode() const {
    return rva_->aggregate_mode();
}

std::list<SpawnPoint> GridResource::spawn_points() const {
    return {};
}

std::map<WayPointLocation, PointsAndAdjacency<double, 3>> GridResource::way_points() const
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
