#include "Triangle_List.hpp"
#include <Mlib/Geometry/Delaunay.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Ambient_Occlusion_By_Curvature.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Geometry/Static_Face_Lighting.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Geometry/Triangle_Tangent.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Thread_Top.hpp>
#include <unordered_map>
#include <unordered_set>

using namespace Mlib;

template <class TPos>
TriangleList<TPos>::TriangleList(
    GroupAndName name,
    const Material& material,
    const Morphology& morphology,
    UUList<FixedArray<ColoredVertex<TPos>, 4>>&& quads,
    UUList<FixedArray<ColoredVertex<TPos>, 3>>&& triangles,
    UUList<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
    UUList<FixedArray<uint8_t, 3>>&& discrete_triangle_texture_layers,
    UUList<FixedArray<float, 3>>&& alpha,
    UUList<FixedArray<float, 4>>&& interiormap_uvmaps)
    : name{ std::move(name) }
    , material{ material }
    , morphology{ morphology }
    , quads{ std::move(quads) }
    , triangles{ std::move(triangles) }
    , triangle_bone_weights{ std::move(triangle_bone_weights) }
    , discrete_triangle_texture_layers{ std::move(discrete_triangle_texture_layers) }
    , alpha{ std::move(alpha) }
    , interiormap_uvmaps{ std::move(interiormap_uvmaps) }
{}

template <class TPos>
void TriangleList<TPos>::draw_triangle_with_normals(
    const FixedArray<TPos, 3>& p00,
    const FixedArray<TPos, 3>& p10,
    const FixedArray<TPos, 3>& p01,
    const FixedArray<float, 3>& n00,
    const FixedArray<float, 3>& n10,
    const FixedArray<float, 3>& n01,
    const FixedArray<uint8_t, 4>& c00,
    const FixedArray<uint8_t, 4>& c10,
    const FixedArray<uint8_t, 4>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u01,
    const std::optional<FixedArray<float, 4>>& interiormap_uvmap,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b01,
    TriangleTangentErrorBehavior tangent_error_behavior,
    ColoredVertex<TPos>** pp00,
    ColoredVertex<TPos>** pp10,
    ColoredVertex<TPos>** pp01)
{
    using I = funpack_t<TPos>;
    ColoredVertex<TPos> v00{ p00, c00, u00, n00 };
    ColoredVertex<TPos> v10{ p10, c10, u10, n10 };
    ColoredVertex<TPos> v01{ p01, c01, u01, n01 };

    FixedArray<I, 3> tangent = triangle_tangent(
        funpack(v00.position),
        funpack(v10.position),
        funpack(v01.position),
        v00.uv.template casted<I>(),
        v10.uv.template casted<I>(),
        v01.uv.template casted<I>(),
        tangent_error_behavior);
    v00.tangent = tangent.template casted<float>();
    v10.tangent = tangent.template casted<float>();
    v01.tangent = tangent.template casted<float>();

    auto& triangle = triangles.emplace_back(v00, v10, v01);
    if (!b00.empty() || !b10.empty() || !b01.empty()) {
        triangle_bone_weights.emplace_back(b00, b10, b01);
        if (triangles.size() != triangle_bone_weights.size()) {
            THROW_OR_ABORT("Triangle bone size mismatch");
        }
    }
    if (interiormap_uvmap.has_value()) {
        interiormap_uvmaps.emplace_back(*interiormap_uvmap);
        if (triangles.size() != interiormap_uvmaps.size()) {
            THROW_OR_ABORT("Interiormap uscale size mismatch");
        }
    }
    if (pp00 != nullptr) {
        *pp00 = &triangle(0);
    }
    if (pp10 != nullptr) {
        *pp10 = &triangle(1);
    }
    if (pp01 != nullptr) {
        *pp01 = &triangle(2);
    }
}

template <class TPos>
void TriangleList<TPos>::draw_triangle_wo_normals(
    const FixedArray<TPos, 3>& p00,
    const FixedArray<TPos, 3>& p10,
    const FixedArray<TPos, 3>& p01,
    const FixedArray<uint8_t, 4>& c00,
    const FixedArray<uint8_t, 4>& c10,
    const FixedArray<uint8_t, 4>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u01,
    const std::optional<FixedArray<float, 4>>& interiormap_uvmap,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b01,
    NormalVectorErrorBehavior normal_error_behavior,
    TriangleTangentErrorBehavior tangent_error_behavior,
    ColoredVertex<TPos>** pp00,
    ColoredVertex<TPos>** pp10,
    ColoredVertex<TPos>** pp01)
{
    using I = funpack_t<TPos>;
    auto t = FixedArray<TPos, 3, 3>{ p00, p10, p01 };
    auto n_o = try_triangle_normal(t.template casted<I>());
    if (!n_o.has_value()) {
        if (any(normal_error_behavior & NormalVectorErrorBehavior::SKIP)) {
            if (any(normal_error_behavior & NormalVectorErrorBehavior::WARN)) {
                lwarn() << "Cannot calculate triangle normal";
            }
            if (pp00 != nullptr) {
                *pp00 = nullptr;
            }
            if (pp10 != nullptr) {
                *pp10 = nullptr;
            }
            if (pp01 != nullptr) {
                *pp01 = nullptr;
            }
            return;
        }
        n_o = get_alternative_or_throw<I>(t, normal_error_behavior);
    }
    auto n = n_o->template casted<float>();
    draw_triangle_with_normals(
        p00, p10, p01, n, n, n, c00, c10, c01, u00, u10, u01, interiormap_uvmap,
        b00, b10, b01, tangent_error_behavior, pp00, pp10, pp01);
}

template <class TPos>
void TriangleList<TPos>::draw_rectangle_with_normals(
    const FixedArray<TPos, 3>& p00,
    const FixedArray<TPos, 3>& p10,
    const FixedArray<TPos, 3>& p11,
    const FixedArray<TPos, 3>& p01,
    const FixedArray<float, 3>& n00,
    const FixedArray<float, 3>& n10,
    const FixedArray<float, 3>& n11,
    const FixedArray<float, 3>& n01,
    const FixedArray<uint8_t, 4>& c00,
    const FixedArray<uint8_t, 4>& c10,
    const FixedArray<uint8_t, 4>& c11,
    const FixedArray<uint8_t, 4>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u11,
    const FixedArray<float, 2>& u01,
    const std::optional<FixedArray<float, 4>>& interiormap_uvmap,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b11,
    const std::vector<BoneWeight>& b01,
    TriangleTangentErrorBehavior tangent_error_behavior,
    RectangleTriangulationMode rectangle_triangulation_mode,
    DelaunayErrorBehavior delaunay_error_behavior,
    ColoredVertex<TPos>** pp00a,
    ColoredVertex<TPos>** pp11a,
    ColoredVertex<TPos>** pp01a, 
    ColoredVertex<TPos>** pp00b,
    ColoredVertex<TPos>** pp10b,
    ColoredVertex<TPos>** pp11b)
{
    DelaunayState delaunay_state;
    if (rectangle_triangulation_mode == RectangleTriangulationMode::DELAUNAY) {
        delaunay_state = is_delaunay(
            funpack(p00),
            funpack(p10),
            funpack(p11),
            funpack(p01));
        if ((delaunay_state == DelaunayState::DELAUNAY) ||
            (delaunay_state == DelaunayState::NOT_DELAUNAY))
        {
            // Do nothing
        } else if (delaunay_state != DelaunayState::ERROR) {
            THROW_OR_ABORT("Unknown Delaunay state: " + std::to_string((int)delaunay_state));
        } else if (delaunay_error_behavior == DelaunayErrorBehavior::SKIP) {
            return;
        } else if (delaunay_error_behavior == DelaunayErrorBehavior::WARN) {
            lwarn() << "Delaunay error";
        } else if (delaunay_error_behavior == DelaunayErrorBehavior::THROW) {
            THROW_OR_ABORT("Delaunay error");
        } else {
            THROW_OR_ABORT("Unknown Delaunay error behavior: " + std::to_string((int)delaunay_error_behavior));
        }
    }
    if ((rectangle_triangulation_mode == RectangleTriangulationMode::FIRST) ||
        ((rectangle_triangulation_mode == RectangleTriangulationMode::DELAUNAY) &&
            (delaunay_state == DelaunayState::DELAUNAY)))
    {
        draw_triangle_with_normals(p00, p11, p01, n00, n11, n01, c00, c11, c01, u00, u11, u01, interiormap_uvmap, b00, b11, b01, tangent_error_behavior, pp00a, pp11a, pp01a);
        draw_triangle_with_normals(p00, p10, p11, n00, n10, n11, c00, c10, c11, u00, u10, u11, interiormap_uvmap, b00, b10, b11, tangent_error_behavior, pp00b, pp10b, pp11b);
    } else if (rectangle_triangulation_mode == RectangleTriangulationMode::DELAUNAY) {
        if (pp00a || pp11a || pp01a || pp00b || pp10b || pp11b) {
            THROW_OR_ABORT("Triangle positions not supported for Delaunay flipping");
        }
        draw_triangle_with_normals(p01, p10, p11, n01, n10, n11, c01, c10, c11, u01, u10, u11, interiormap_uvmap, b01, b10, b11, tangent_error_behavior);
        draw_triangle_with_normals(p00, p10, p01, n00, n10, n01, c00, c10, c01, u00, u10, u01, interiormap_uvmap, b00, b10, b01, tangent_error_behavior);
    } else if (rectangle_triangulation_mode == RectangleTriangulationMode::DISABLED) {
        if (pp00a || pp11a || pp01a || pp00b || pp10b || pp11b) {
            THROW_OR_ABORT("Triangle positions not supported for quads");
        }
        quads.emplace_back(
            ColoredVertex<TPos>{
                p00,
                c00,
                u00,
                n00},
            ColoredVertex<TPos>{
                p10,
                c10,
                u10,
                n10},
            ColoredVertex<TPos>{
                p11,
                c11,
                u11,
                n11},
            ColoredVertex<TPos>{
                p01,
                c01,
                u01,
                n01});
    } else {
        THROW_OR_ABORT("Unsupported triangulation mode (0)");
    }
}

template <class TPos>
void TriangleList<TPos>::draw_rectangle_wo_normals(
    const FixedArray<TPos, 3>& p00,
    const FixedArray<TPos, 3>& p10,
    const FixedArray<TPos, 3>& p11,
    const FixedArray<TPos, 3>& p01,
    const FixedArray<uint8_t, 4>& c00,
    const FixedArray<uint8_t, 4>& c10,
    const FixedArray<uint8_t, 4>& c11,
    const FixedArray<uint8_t, 4>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u11,
    const FixedArray<float, 2>& u01,
    const std::optional<FixedArray<float, 4>>& interiormap_uvmap,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b11,
    const std::vector<BoneWeight>& b01,
    NormalVectorErrorBehavior normal_error_behavior,
    TriangleTangentErrorBehavior tangent_error_behavior,
    RectangleTriangulationMode rectangle_triangulation_mode,
    DelaunayErrorBehavior delaunay_error_behavior,
    ColoredVertex<TPos>** pp00a,
    ColoredVertex<TPos>** pp11a,
    ColoredVertex<TPos>** pp01a,
    ColoredVertex<TPos>** pp00b,
    ColoredVertex<TPos>** pp10b,
    ColoredVertex<TPos>** pp11b)
{
    DelaunayState delaunay_state;
    if (rectangle_triangulation_mode == RectangleTriangulationMode::DELAUNAY) {
        delaunay_state = is_delaunay(
            funpack(p00),
            funpack(p10),
            funpack(p11),
            funpack(p01));
        if ((delaunay_state == DelaunayState::DELAUNAY) ||
            (delaunay_state == DelaunayState::NOT_DELAUNAY))
        {
            // Do nothing
        } else if (delaunay_state != DelaunayState::ERROR) {
            THROW_OR_ABORT("Unknown Delaunay state: " + std::to_string((int)delaunay_state));
        } else if (delaunay_error_behavior == DelaunayErrorBehavior::SKIP) {
            return;
        } else if (delaunay_error_behavior == DelaunayErrorBehavior::WARN) {
            lwarn() << "Delaunay error";
        } else if (delaunay_error_behavior == DelaunayErrorBehavior::THROW) {
            THROW_OR_ABORT("Delaunay error");
        } else {
            THROW_OR_ABORT("Unknown Delaunay error behavior: " + std::to_string((int)delaunay_error_behavior));
        }
    }
    if ((rectangle_triangulation_mode == RectangleTriangulationMode::FIRST) ||
        ((rectangle_triangulation_mode == RectangleTriangulationMode::DELAUNAY) &&
            (delaunay_state == DelaunayState::DELAUNAY)))
    {
        draw_triangle_wo_normals(p00, p11, p01, c00, c11, c01, u00, u11, u01, interiormap_uvmap, b00, b11, b01, normal_error_behavior, tangent_error_behavior, pp00a, pp11a, pp01a);
        draw_triangle_wo_normals(p00, p10, p11, c00, c10, c11, u00, u10, u11, interiormap_uvmap, b00, b10, b11, normal_error_behavior, tangent_error_behavior, pp00b, pp10b, pp11b);
    } else if (rectangle_triangulation_mode == RectangleTriangulationMode::DELAUNAY) {
        if (pp00a || pp11a || pp01a || pp00b || pp10b || pp11b) {
            THROW_OR_ABORT("Triangle positions not supported for Delaunay flipping");
        }
        draw_triangle_wo_normals(p01, p10, p11, c01, c10, c11, u01, u10, u11, interiormap_uvmap, b01, b10, b11, normal_error_behavior, tangent_error_behavior);
        draw_triangle_wo_normals(p00, p10, p01, c00, c10, c01, u00, u10, u01, interiormap_uvmap, b00, b10, b01, normal_error_behavior, tangent_error_behavior);
    } else {
        THROW_OR_ABORT("Unsupported triangulation mode (1)");
    }
}

template <class TPos>
void TriangleList<TPos>::extrude(
    TriangleList& dest,
    const std::list<std::shared_ptr<TriangleList>>& triangle_lists,
    const std::list<std::shared_ptr<TriangleList>>* follower_triangles,
    const std::list<std::shared_ptr<TriangleList>>* source_triangles,
    const std::set<OrderableFixedArray<TPos, 3>>* clamped_vertices,
    const std::set<OrderableFixedArray<TPos, 3>>* vertices_not_to_connect,
    TPos height,
    float scale,
    float uv_scale_x,
    float uv_scale_y,
    bool uvs_equal_lengths,
    float ambient_occlusion)
{
    float old_ambient_occlusion = (height > (TPos)0.)
        ? ambient_occlusion
        : 0.f;
    float new_ambient_occlusion = (height < (TPos)0.)
        ? ambient_occlusion
        : 0.f;
    using O = OrderableFixedArray<TPos, 3>;

    // Only relevant if "source_triangles != nullptr".
    // Map "edge -> vertices" for different uv-coordinates.
    std::unordered_map<std::pair<O, O>, std::pair<const ColoredVertex<TPos>*, const ColoredVertex<TPos>*>> edge_map;
    if (source_triangles != nullptr) {
        for (const auto& l : *source_triangles) {
            for (const auto& t : l->triangles) {
                edge_map[{O{t(1).position}, O{t(0).position}}] = {&t(1), &t(0)};
                edge_map[{O{t(2).position}, O{t(1).position}}] = {&t(2), &t(1)};
                edge_map[{O{t(0).position}, O{t(2).position}}] = {&t(0), &t(2)};
            }
        }
    }

    std::list<FixedArray<ColoredVertex<TPos>, 3>*> tris;
    for (auto& l : triangle_lists) {
        for (auto& t : l->triangles) {
            tris.push_back(&t);
        }
    }
    std::unordered_set<std::pair<O, O>> contour_edges = find_contour_edges(tris);
    std::unordered_set<O> moved_vertices;
    for (auto& t : tris) {
        FixedArray<ColoredVertex<TPos>, 3> t_old = *t;
        FixedArray<bool, 3> is_clamped{
            clamped_vertices != nullptr && clamped_vertices->contains(make_orderable((*t)(0).position)),
            clamped_vertices != nullptr && clamped_vertices->contains(make_orderable((*t)(1).position)),
            clamped_vertices != nullptr && clamped_vertices->contains(make_orderable((*t)(2).position))};
        FixedArray<bool, 3> not_to_connect{
            (vertices_not_to_connect != nullptr) && vertices_not_to_connect->contains(make_orderable((*t)(0).position)),
            (vertices_not_to_connect != nullptr) && vertices_not_to_connect->contains(make_orderable((*t)(1).position)),
            (vertices_not_to_connect != nullptr) && vertices_not_to_connect->contains(make_orderable((*t)(2).position))};
        auto connect_extruded = [&](size_t a, size_t b){
            if (is_clamped(a) && is_clamped(b)) {
                return;
            }
            if (not_to_connect(a) && not_to_connect(b)) {
                return;
            }
            auto edge = std::make_pair(O{t_old(a).position}, O{t_old(b).position});
            if (!contour_edges.contains(edge)) {
                return;
            }
            const ColoredVertex<TPos>* va;
            const ColoredVertex<TPos>* vb;
            if (source_triangles != nullptr) {
                auto it = edge_map.find(edge);
                if (it == edge_map.end()) {
                    return;
                } else {
                    va = it->second.first;
                    vb = it->second.second;
                }
            } else {
                va = &t_old(a);
                vb = &t_old(b);
            }
            auto duv1 = (float)(funpack(height) / scale * uv_scale_y);
            auto duv = FixedArray<float, 2>{ 0.f, duv1 };
            if (va->uv(0) == vb->uv(0)) {
                std::swap(duv(0), duv(1));
            }
            // Already checked above:
            // (is_clamped(a) && is_clamped(b))
            if (is_clamped(a)) {
                dest.draw_triangle_wo_normals(
                    va->position,
                    vb->position,
                    (*t)(b).position,
                    Colors::scale(va->color, 1.f - old_ambient_occlusion),
                    Colors::scale(vb->color, 1.f - old_ambient_occlusion),
                    Colors::scale(vb->color, 1.f - new_ambient_occlusion),
                    va->uv,
                    vb->uv,
                    vb->uv + duv);
            } else if (is_clamped(b)) {
                dest.draw_triangle_wo_normals(
                    va->position,
                    vb->position,
                    (*t)(a).position,
                    Colors::scale(va->color, 1.f - old_ambient_occlusion),
                    Colors::scale(vb->color, 1.f - old_ambient_occlusion),
                    Colors::scale(va->color, 1.f - new_ambient_occlusion),
                    va->uv,
                    vb->uv,
                    va->uv + duv);
            } else {
                auto len = (float)std::sqrt(sum(squared(vb->position - va->position)));
                dest.draw_rectangle_wo_normals(
                    va->position,
                    vb->position,
                    (*t)(b).position,
                    (*t)(a).position,
                    Colors::scale(va->color, 1.f - old_ambient_occlusion),
                    Colors::scale(vb->color, 1.f - old_ambient_occlusion),
                    Colors::scale(vb->color, 1.f - new_ambient_occlusion),
                    Colors::scale(va->color, 1.f - new_ambient_occlusion),
                    uvs_equal_lengths ? FixedArray<float, 2>{0.f, 0.f} : va->uv,
                    uvs_equal_lengths ? FixedArray<float, 2>{len / scale * uv_scale_x, 0.f} : vb->uv,
                    uvs_equal_lengths ? FixedArray<float, 2>{len / scale * uv_scale_x, duv1} : vb->uv + duv,
                    uvs_equal_lengths ? FixedArray<float, 2>{0.f, duv1} : va->uv + duv);
            }
        };
        for (size_t i = 0; i < 3; ++i) {
            if (!is_clamped(i)) {
                if (follower_triangles != nullptr) {
                    moved_vertices.insert(O{(*t)(i).position});
                }
                (*t)(i).position(2) += height;
                if (!uvs_equal_lengths) {
                    (*t)(i).uv(0) *= uv_scale_x;
                }
            }
        }

        connect_extruded(0, 1);
        connect_extruded(1, 2);
        connect_extruded(2, 0);
    }
    if (follower_triangles != nullptr) {
        for (auto& lst : *follower_triangles) {
            for (auto& t : lst->triangles) {
                for (auto& v : t.flat_iterable()) {
                    if (moved_vertices.contains(O{v.position})) {
                        v.position(2) += (TPos)height;
                    }
                }
            }
        }
    }
}

template <class TPos>
std::list<std::shared_ptr<TriangleList<TPos>>> TriangleList<TPos>::concatenated(
    const std::list<std::shared_ptr<TriangleList>>& a,
    const std::list<std::shared_ptr<TriangleList>>& b)
{
    std::list<std::shared_ptr<TriangleList>> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

template <class TPos>
void TriangleList<TPos>::delete_backfacing_triangles(
    std::list<FixedArray<ColoredVertex<TPos>, 3>>* deleted_triangles)
{
    using I = funpack_t<TPos>;
    triangles.remove_if([deleted_triangles](const FixedArray<ColoredVertex<TPos>, 3>& t) -> bool{
        bool erase = dot0d(scaled_triangle_normal<I>({
                funpack(t(0).position),
                funpack(t(1).position),
                funpack(t(2).position)}),
            FixedArray<I, 3>{I(0), I(0), I(1)}) <= 0;
        if (erase) {
            if (deleted_triangles != nullptr) {
                deleted_triangles->push_back(t);
            }
            // lerr() << "Triangle at has negative normal direction";
            // draw_node(*triangles, {t(0).position(0), t(0).position(1)}, scale * 5);
            // draw_node(*triangles, {t(1).position(0), t(1).position(1)}, scale * 5);
            // draw_node(*triangles, {t(2).position(0), t(2).position(1)}, scale * 5);
        }
        return erase;
    });
}

template <class TPos>
void TriangleList<TPos>::calculate_triangle_normals(NormalVectorErrorBehavior error_behavior) {
    for (auto& t : triangles) {
        using Triangle = FixedArray<TPos, 3, 3>;
        auto n = triangle_normal(funpack(Triangle{t(0).position, t(1).position, t(2).position}), error_behavior);
        t(0).normal = n.template casted<float>();
        t(1).normal = n.template casted<float>();
        t(2).normal = n.template casted<float>();
    }
}

template <class TPos>
void TriangleList<TPos>::convert_triangle_to_vertex_normals() {
    VertexNormals<TPos, float> vertex_normals;
    vertex_normals.add_triangles(triangles.begin(), triangles.end());
    vertex_normals.compute_vertex_normals(NormalVectorErrorBehavior::THROW);
    for (auto& it : triangles) {
        for (auto& v : it.flat_iterable()) {
            v.normal = vertex_normals.get_normal(v.position);
        }
    }
}

template <class TPos>
void TriangleList<TPos>::flip() {
    for (auto& l : triangles) {
        for (auto& v : l.flat_iterable()) {
            v.normal = -v.normal;
        }
        std::swap(l(0), l(1));
    }
}

template <class TPos>
void TriangleList<TPos>::convert_triangle_to_vertex_normals(const std::list<std::shared_ptr<TriangleList>>& triangle_lists) {
    VertexNormals<TPos, float> vertex_normals;
    for (const auto& l : triangle_lists) {
        vertex_normals.add_triangles(
            l->triangles.begin(),
            l->triangles.end());
    }
    vertex_normals.compute_vertex_normals(NormalVectorErrorBehavior::THROW);
    for (const auto& l : triangle_lists) {
        for (auto& it : l->triangles) {
            for (auto& v : it.flat_iterable()) {
                v.normal = vertex_normals.get_normal(v.position);
            }
        }
    }
}

template <class TPos>
void TriangleList<TPos>::ambient_occlusion_by_curvature(
    const std::list<std::shared_ptr<TriangleList>>& triangle_lists,
    float strength)
{
    std::list<FixedArray<ColoredVertex<TPos>, 3>*> cvl;
    for (auto& l : triangle_lists) {
        for (auto& t : l->triangles) {
            cvl.push_back(&t);
        }
    }
    Mlib::ambient_occlusion_by_curvature(cvl, strength);
}

template <class TPos>
struct VertexMovement {
    bool move_only_z;
    FixedArray<float, 3> amount = uninitialized;
    FixedArray<float, 3> bias = uninitialized;
    std::list<FixedArray<TPos, 3>*> vertices;
};

template <class TPos>
struct AdjacentTriangles {
    const FixedArray<TPos, 3>* ei;
    const FixedArray<TPos, 3>* ej;
    const FixedArray<TPos, 3>* n0;
    const FixedArray<TPos, 3>* n1;
    FixedArray<float, 3>* movement_i;
    FixedArray<float, 3>* movement_j;
};

template <class TPos>
void TriangleList<TPos>::smoothen_edges(
    std::unordered_map<FixedArray<TPos, 3>*, VertexHeightBinding<TPos>>& vertex_height_bindings,
    const std::unordered_map<OrderableFixedArray<TPos, 3>, FixedArray<float, 3>>& bias,
    const std::list<std::shared_ptr<TriangleList>>& edge_triangle_lists,
    const std::list<std::shared_ptr<TriangleList>>& excluded_triangle_lists,
    const std::list<std::shared_ptr<TriangleList>>& move_only_z_triangle_lists,
    const std::list<FixedArray<TPos, 3>*>& smoothed_vertices,
    float smoothness,
    size_t niterations,
    bool move_only_z,
    float decay)
{
    FunctionGuard fg{ "Smoothen edges" };
    using Vertex2 = OrderableFixedArray<TPos, 2>;
    std::unordered_set<Vertex2> excluded_vertices;
    for (const auto& tl : excluded_triangle_lists) {
        for (const auto& t : tl->triangles) {
            for (const auto& v : t.flat_iterable()) {
                excluded_vertices.insert(Vertex2{v.position(0), v.position(1)});
            }
        }
    }
    std::unordered_set<Vertex2> move_only_z_vertices;
    for (const auto& tl : move_only_z_triangle_lists) {
        for (const auto& t : tl->triangles) {
            for (const auto& v : t.flat_iterable()) {
                move_only_z_vertices.insert(Vertex2{v.position(0), v.position(1)});
            }
        }
    }
    std::unordered_map<Vertex2, VertexMovement<TPos>> vertex_movement;
    for (const auto& s : smoothed_vertices) {
        Vertex2 vc{ uninitialized };
        auto hit = vertex_height_bindings.find(s);
        if (hit != vertex_height_bindings.end()) {
            vc = hit->second.value();
        } else {
            vc = Vertex2{ (*s)(0), (*s)(1) };
        }
        if (excluded_vertices.contains(vc)) {
            continue;
        }
        auto& m = vertex_movement[vc];
        if (m.vertices.empty()) {
            m.move_only_z = move_only_z_vertices.contains(vc);
            m.amount = 0.f;
            auto bit = bias.find(make_orderable(*s));
            m.bias = (bit != bias.end())
                ? bit->second
                : fixed_zeros<float, 3>();
        }
        m.vertices.push_back(s);
    }
    using Vertex3 = FixedArray<TPos, 3>;
    using Edge3 = OrderableFixedArray<TPos, 2, 3>;
    std::unordered_map<Edge3, AdjacentTriangles<TPos>> adjacent_triangles;
    bool warning0_emitted = false;
    bool warning1_emitted = false;
    {
        fg.update("Find adjacent triangles");
        for (const auto& l : edge_triangle_lists) {
            for (const auto& t : l->triangles) {
                auto insert_edge = [&](size_t i, size_t j, size_t n){
                    const auto& ei = t(i).position;
                    const auto& ej = t(j).position;
                    const auto& nn = t(n).position;
                    Vertex2 ei2{ ei(0), ei(1) };
                    Edge3 esi{ make_orderable(*ej), make_orderable(*ei) };
                    Edge3 esj{ make_orderable(*ei), make_orderable(*ej) };
                    auto it = adjacent_triangles.find(esj);
                    if (it == adjacent_triangles.end()) {
                        const auto at = adjacent_triangles.try_emplace(esi, AdjacentTriangles<TPos>{
                            .ei = &ei,
                            .ej = &ej,
                            .n0 = &nn,
                            .n1 = nullptr,
                            .movement_i = excluded_vertices.contains(ei2)
                                ? nullptr
                                : &vertex_movement.at(ei2).amount,
                            .movement_j = nullptr
                        });
                        if (!at.second) {
                            if (!warning0_emitted) {
                                lwarn() << "More than 2 triangles share the same edge, further warnings suppressed (0)";
                                warning0_emitted = true;
                            }
                            return;
                        }
                    } else {
                        if (it->second.n1 != nullptr) {
                            if (!warning1_emitted) {
                                lwarn() << "More than 2 triangles share the same edge, further warnings suppressed (1)";
                                warning1_emitted = true;
                            }
                            return;
                        }
                        it->second.n1 = &nn;
                        if (!excluded_vertices.contains(ei2)) {
                            it->second.movement_j = &vertex_movement.at(ei2).amount;
                        }
                    }
                    };
                insert_edge(0, 1, 2);
                insert_edge(1, 2, 0);
                insert_edge(2, 0, 1);
            }
        }
    }
    std::erase_if(adjacent_triangles, [](const auto& it) {
        return (it.second.n1 == nullptr) || ((it.second.movement_i == nullptr) && (it.second.movement_j == nullptr));
        });
    using I = funpack_t<TPos>;
    for (size_t i = 0; i < niterations; ++i) {
        fg.update("Smoothen edges, iteration " + std::to_string(i) + '/' + std::to_string(niterations));
        for (auto& [_, t] : adjacent_triangles) {
            using Triangle = FixedArray<TPos, 3, 3>;
            const Vertex3& ei = *t.ej;
            const Vertex3& ej = *t.ei;
            const Vertex3& nx = *t.n0;
            const Vertex3& nn = *t.n1;
            FixedArray<float, 3> n0 = triangle_normal<I>(funpack(Triangle{ej, ei, nx})).template casted<float>();
            FixedArray<float, 3> n1 = triangle_normal<I>(funpack(Triangle{ei, ej, nn})).template casted<float>();
            FixedArray<TPos, 3> cn = (nx + nn) / operand<TPos, 2>;
            FixedArray<TPos, 3> ce = (ei + ej) / operand<TPos, 2>;
            FixedArray<float, 3> v = (cn - ce).template casted<float>();
            FixedArray<float, 3> n01 = (n0 + n1) / 2.f;
            n01 /= std::sqrt(sum(squared(n01)));
            float n0n1 = dot0d(n0, n1);
            if (n0n1 >=0 && n0n1 < 1) {
                float shift = std::sqrt(1 - squared(n0n1)) * sign(dot0d(v, n01));
                if (t.movement_i != nullptr) {
                    *t.movement_i += n01 * shift;
                }
                if (t.movement_j != nullptr) {
                    *t.movement_j += n01 * shift;
                }
            }
        }
        for (auto& [_, m] : vertex_movement) {
            for (auto& n : m.vertices) {
                auto dn = (smoothness * (m.amount + m.bias)).template casted<TPos>();
                if (move_only_z || m.move_only_z) {
                    (*n)(2) += dn(2);
                } else {
                    *n += dn;
                }
            }
            m.amount = 0;
        }
        smoothness *= decay;
    }
}

template <class TPos>
std::shared_ptr<ColoredVertexArray<TPos>> TriangleList<TPos>::triangle_array() const {
    return std::make_shared<ColoredVertexArray<TPos>>(
        name,
        material,
        morphology,
        modifier_backlog,
        UUVector<FixedArray<ColoredVertex<TPos>, 4>>(quads.begin(), quads.end()),
        UUVector<FixedArray<ColoredVertex<TPos>, 3>>(triangles.begin(), triangles.end()),
        UUVector<FixedArray<ColoredVertex<TPos>, 2>>(),
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>(triangle_bone_weights.begin(), triangle_bone_weights.end()),
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<uint8_t, 3>>(discrete_triangle_texture_layers.begin(), discrete_triangle_texture_layers.end()),
        std::vector<UUVector<FixedArray<float, 3, 2>>>{},
        std::vector<UUVector<FixedArray<float, 3>>>{},
        std::vector(alpha.begin(), alpha.end()),
        std::vector(interiormap_uvmaps.begin(), interiormap_uvmaps.end()));
}

namespace Mlib {

template class TriangleList<float>;
template class TriangleList<CompressedScenePos>;

}
