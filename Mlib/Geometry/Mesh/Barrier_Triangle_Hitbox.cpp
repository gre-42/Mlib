#include "Barrier_Triangle_Hitbox.hpp"
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

template <class TPos>
std::vector<FixedArray<FixedArray<TPos, 3>, 3>> Mlib::barrier_triangle_hitbox(
    const FixedArray<TPos, 3>& am,
    const FixedArray<TPos, 3>& bm,
    const FixedArray<TPos, 3>& cm,
    const FixedArray<float, 3>& half_width_a,
    const FixedArray<float, 3>& half_width_b,
    const FixedArray<float, 3>& half_width_c,
    bool ab_is_contour_edge,
    bool bc_is_contour_edge,
    bool ca_is_contour_edge)
{
    FixedArray<TPos, 3> A = am + half_width_a.casted<TPos>();
    FixedArray<TPos, 3> B = bm + half_width_b.casted<TPos>();
    FixedArray<TPos, 3> C = cm + half_width_c.casted<TPos>();
    FixedArray<TPos, 3> a = am - half_width_a.casted<TPos>();
    FixedArray<TPos, 3> b = bm - half_width_b.casted<TPos>();
    FixedArray<TPos, 3> c = cm - half_width_c.casted<TPos>();
    if ((dot0d(bm - am, B - A) <= 0) ||
        (dot0d(cm - bm, C - B) <= 0) ||
        (dot0d(am - cm, A - C) <= 0) ||
        (dot0d(bm - am, b - a) <= 0) ||
        (dot0d(cm - bm, c - b) <= 0) ||
        (dot0d(am - cm, a - c) <= 0))
    {
        throw TriangleException<TPos>{am, bm, cm, "convex_decomposition_terrain: consistency-check failed"};
    }
    std::vector<FixedArray<FixedArray<TPos, 3>, 3>> result;
    result.reserve(
        ab_is_contour_edge * 2 +
        bc_is_contour_edge * 2 +
        ca_is_contour_edge * 2);

    result.push_back({a, b, c});
    if (ab_is_contour_edge) {
        result.push_back({a, A, b});
        result.push_back({A, B, b});
    }
    if (bc_is_contour_edge) {
        result.push_back({b, B, c});
        result.push_back({B, C, c});
    }
    if (ca_is_contour_edge) {
        result.push_back({c, C, a});
        result.push_back({C, A, a});
    }
    result.push_back({B, A, C});
    return result;
}

template <class TPos>
std::vector<std::shared_ptr<ColoredVertexArray<TPos>>> Mlib::create_barrier_triangle_hitboxes(
    const ColoredVertexArray<TPos>& cva,
    float half_width,
    PhysicsMaterial destination_physics_material)
{
    if (!any(cva.physics_material & PhysicsMaterial::ATTR_TWO_SIDED)) {
        THROW_OR_ABORT("Terrain not marked as two-sided");
    }
    if (!any(cva.physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
        THROW_OR_ABORT("Terrain to be decomposed is not collidable");
    }
    if (!any(destination_physics_material & PhysicsMaterial::ATTR_CONCAVE)) {
        THROW_OR_ABORT("Destination mesh is not tagged as concave");
    }
    std::list<const FixedArray<ColoredVertex<TPos>, 3>*> triangle_ptrs;
    for (const auto& t : cva.triangles) {
        triangle_ptrs.push_back(&t);
    }
    auto contour_edges = find_contour_edges(triangle_ptrs);

    VertexNormals<TPos, float> vertex_normals;
    vertex_normals.add_triangles(cva.triangles.begin(), cva.triangles.end());
    vertex_normals.compute_vertex_normals();

    std::vector<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    result.reserve(cva.triangles.size() + 1);
    result.push_back(
        std::make_shared<ColoredVertexArray<TPos>>(
            cva.name + "_visual",
            cva.material,
            cva.physics_material & ~PhysicsMaterial::ATTR_COLLIDE,
            std::vector{cva.triangles},
            std::vector{cva.lines},
            std::vector{cva.triangle_bone_weights},
            std::vector{cva.line_bone_weights},
            std::vector{cva.triangle_texture_layers},
            std::vector{cva.line_texture_layers}));
    std::vector<FixedArray<ColoredVertex<TPos>, 3>> decomposition;
    decomposition.reserve(2 * cva.triangles.size() + 2 * contour_edges.size());
    for (const auto& tri : cva.triangles) {
        auto hitbox = barrier_triangle_hitbox(
            tri(0).position,
            tri(1).position,
            tri(2).position,
            vertex_normals.get_normal(tri(0).position) * (-half_width),
            vertex_normals.get_normal(tri(1).position) * (-half_width),
            vertex_normals.get_normal(tri(2).position) * (-half_width),
            contour_edges.contains({OrderableFixedArray{tri(0).position}, OrderableFixedArray{tri(1).position}}),
            contour_edges.contains({OrderableFixedArray{tri(1).position}, OrderableFixedArray{tri(2).position}}),
            contour_edges.contains({OrderableFixedArray{tri(2).position}, OrderableFixedArray{tri(0).position}}));
        for (const auto& s : hitbox) {
            const auto purple = FixedArray<float, 3>{1.f, 0.f, 1.f};
            const auto zeros2 = fixed_zeros<float, 2>();
            const auto zeros3 = fixed_zeros<float, 3>();
            if (decomposition.capacity() == decomposition.size()) {
                THROW_OR_ABORT("create_barrier_triangle_hitboxes internal error (0)");
            }
            auto n = triangle_normal(s) TEMPLATEV casted<float>();
            decomposition.push_back({
                ColoredVertex<TPos>{.position = s(0), .color = purple, .uv = zeros2, .normal = n, .tangent = zeros3},
                ColoredVertex<TPos>{.position = s(1), .color = purple, .uv = zeros2, .normal = n, .tangent = zeros3},
                ColoredVertex<TPos>{.position = s(2), .color = purple, .uv = zeros2, .normal = n, .tangent = zeros3}});
        }
    }
    if (decomposition.capacity() != decomposition.size()) {
        THROW_OR_ABORT("create_barrier_triangle_hitboxes internal error (1)");
    }
    auto removed_attributes =
        PhysicsMaterial::ATTR_VISIBLE |
        PhysicsMaterial::ATTR_COLLIDE |
        PhysicsMaterial::ATTR_TWO_SIDED |
        PhysicsMaterial::ATTR_CONVEX |
        PhysicsMaterial::ATTR_CONCAVE;
    result.push_back(
        std::make_shared<ColoredVertexArray<TPos>>(
            cva.name + "_blk",
            Material{
                .aggregate_mode = AggregateMode::ONCE
            },
            destination_physics_material | (cva.physics_material & ~removed_attributes),
            std::move(decomposition),
            std::vector<FixedArray<ColoredVertex<TPos>, 2>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},
            std::vector<FixedArray<uint8_t, 3>>{},
            std::vector<FixedArray<uint8_t, 2>>{}));
    return result;
}

template
std::vector<std::shared_ptr<ColoredVertexArray<double>>> Mlib::create_barrier_triangle_hitboxes(
    const ColoredVertexArray<double>& cva,
    float width,
    PhysicsMaterial destination_physics_material);
