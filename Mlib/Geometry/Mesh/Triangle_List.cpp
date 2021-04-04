#include "Triangle_List.hpp"
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Static_Face_Lightning.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Geometry/Triangle_Tangent.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <map>

using namespace Mlib;

void TriangleList::draw_triangle_with_normals(
    const FixedArray<float, 3>& p00,
    const FixedArray<float, 3>& p10,
    const FixedArray<float, 3>& p01,
    const FixedArray<float, 3>& n00,
    const FixedArray<float, 3>& n10,
    const FixedArray<float, 3>& n01,
    const FixedArray<float, 3>& c00,
    const FixedArray<float, 3>& c10,
    const FixedArray<float, 3>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u01,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b01)
{
    ColoredVertex v00{p00, c00, u00, n00};
    ColoredVertex v10{p10, c10, u10, n10};
    ColoredVertex v01{p01, c01, u01, n01};

    FixedArray<float, 3> tangent = triangle_tangent(
        v00.position,
        v10.position,
        v01.position,
        v00.uv,
        v10.uv,
        v01.uv);
    v00.tangent = tangent;
    v10.tangent = tangent;
    v01.tangent = tangent;

    triangles_.push_back(FixedArray<ColoredVertex, 3>{v00, v10, v01});
    if (!b00.empty() || !b10.empty() || !b01.empty()) {
        triangle_bone_weights_.push_back(FixedArray<std::vector<BoneWeight>, 3>{b00, b10, b01});
        if (triangles_.size() != triangle_bone_weights_.size()) {
            throw std::runtime_error("Triangle bone size mismatch");
        }
    }
}

void TriangleList::draw_triangle_wo_normals(
    const FixedArray<float, 3>& p00,
    const FixedArray<float, 3>& p10,
    const FixedArray<float, 3>& p01,
    const FixedArray<float, 3>& c00,
    const FixedArray<float, 3>& c10,
    const FixedArray<float, 3>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u01,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b01,
    TriangleNormalErrorBehavior normal_error_behavior)
{
    auto n = triangle_normal({p00, p10, p01}, normal_error_behavior);
    draw_triangle_with_normals(p00, p10, p01, n, n, n, c00, c10, c01, u00, u10, u01, b00, b10, b01);
}

void TriangleList::draw_rectangle_with_normals(
    const FixedArray<float, 3>& p00,
    const FixedArray<float, 3>& p10,
    const FixedArray<float, 3>& p11,
    const FixedArray<float, 3>& p01,
    const FixedArray<float, 3>& n00,
    const FixedArray<float, 3>& n10,
    const FixedArray<float, 3>& n11,
    const FixedArray<float, 3>& n01,
    const FixedArray<float, 3>& c00,
    const FixedArray<float, 3>& c10,
    const FixedArray<float, 3>& c11,
    const FixedArray<float, 3>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u11,
    const FixedArray<float, 2>& u01,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b11,
    const std::vector<BoneWeight>& b01)
{
    draw_triangle_with_normals(p00, p11, p01, n00, n11, n01, c00, c11, c01, u00, u11, u01, b00, b11, b01);
    draw_triangle_with_normals(p00, p10, p11, n00, n10, n11, c00, c10, c11, u00, u10, u11, b00, b10, b11);
}

void TriangleList::draw_rectangle_wo_normals(
    const FixedArray<float, 3>& p00,
    const FixedArray<float, 3>& p10,
    const FixedArray<float, 3>& p11,
    const FixedArray<float, 3>& p01,
    const FixedArray<float, 3>& c00,
    const FixedArray<float, 3>& c10,
    const FixedArray<float, 3>& c11,
    const FixedArray<float, 3>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u11,
    const FixedArray<float, 2>& u01,
    const std::vector<BoneWeight>& b00,
    const std::vector<BoneWeight>& b10,
    const std::vector<BoneWeight>& b11,
    const std::vector<BoneWeight>& b01,
    TriangleNormalErrorBehavior normal_error_behavior)
{
    draw_triangle_wo_normals(p00, p11, p01, c00, c11, c01, u00, u11, u01, b00, b11, b01, normal_error_behavior);
    draw_triangle_wo_normals(p00, p10, p11, c00, c10, c11, u00, u10, u11, b00, b10, b11, normal_error_behavior);
}

void TriangleList::extrude(
    TriangleList& dest,
    const std::list<std::shared_ptr<TriangleList>>& triangle_lists,
    const std::list<std::shared_ptr<TriangleList>>* source_triangles,
    const std::set<OrderableFixedArray<float, 3>>* clamped_vertices,
    float height,
    float scale,
    float uv_scale_x,
    float uv_scale_y)
{
    using O = OrderableFixedArray<float, 3>;

    // Only relevant if "source_triangles != nullptr".
    // Map "edge -> vertices" for different uv-coordinates.
    std::map<std::pair<O, O>, std::pair<const ColoredVertex*, const ColoredVertex*>> edge_map;
    if (source_triangles != nullptr) {
        for (const auto& l : *source_triangles) {
            for (const auto& t : l->triangles_) {
                edge_map[{O{t(1).position}, O{t(0).position}}] = {&t(1), &t(0)};
                edge_map[{O{t(2).position}, O{t(1).position}}] = {&t(2), &t(1)};
                edge_map[{O{t(0).position}, O{t(2).position}}] = {&t(0), &t(2)};
            }
        }
    }

    std::list<FixedArray<ColoredVertex, 3>*> tris;
    for (auto& l : triangle_lists) {
        for (auto& t : l->triangles_) {
            tris.push_back(&t);
        }
    }
    std::set<std::pair<O, O>> contour_edges = find_contour_edges(tris);
    for (auto& t : tris) {
        FixedArray<ColoredVertex, 3> t_old = *t;
        FixedArray<bool, 3> is_clamped{
            clamped_vertices != nullptr && clamped_vertices->contains(OrderableFixedArray{(*t)(0).position}),
            clamped_vertices != nullptr && clamped_vertices->contains(OrderableFixedArray{(*t)(1).position}),
            clamped_vertices != nullptr && clamped_vertices->contains(OrderableFixedArray{(*t)(2).position})};
        auto connect_extruded = [&](size_t a, size_t b){
            if (is_clamped(a) && is_clamped(b)) {
                return;
            }
            auto edge = std::make_pair(O{t_old(a).position}, O{t_old(b).position});
            if (!contour_edges.contains(edge)) {
                return;
            }
            const ColoredVertex* va;
            const ColoredVertex* vb;
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
            auto duv = FixedArray<float, 2>{height / scale * uv_scale_y, 0.f};
            if (va->uv(0) != vb->uv(0)) {
                duv = FixedArray<float, 2>{duv(1), duv(0)};
            }
            // Already checked above:
            // (is_clamped(a) && is_clamped(b))
            if (is_clamped(a)) {
                dest.draw_triangle_wo_normals(
                    va->position,
                    vb->position,
                    (*t)(b).position,
                    va->color,
                    vb->color,
                    vb->color,
                    va->uv,
                    vb->uv,
                    vb->uv + duv);
            } else if (is_clamped(b)) {
                dest.draw_triangle_wo_normals(
                    va->position,
                    vb->position,
                    (*t)(a).position,
                    va->color,
                    vb->color,
                    va->color,
                    va->uv,
                    vb->uv,
                    va->uv + duv);
            } else {
                dest.draw_rectangle_wo_normals(
                    va->position,
                    vb->position,
                    (*t)(b).position,
                    (*t)(a).position,
                    va->color,
                    vb->color,
                    vb->color,
                    va->color,
                    va->uv,
                    vb->uv,
                    vb->uv + duv,
                    va->uv + duv);
            }
        };
        for (size_t i = 0; i < 3; ++i) {
            if (!is_clamped(i)) {
                (*t)(i).position(2) += height;
                (*t)(i).uv(0) *= uv_scale_x;
            }
        }

        connect_extruded(0, 1);
        connect_extruded(1, 2);
        connect_extruded(2, 0);
    }
}

std::list<std::shared_ptr<TriangleList>> TriangleList::concatenated(
    const std::list<std::shared_ptr<TriangleList>>& a,
    const std::list<std::shared_ptr<TriangleList>>& b)
{
    std::list<std::shared_ptr<TriangleList>> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

void TriangleList::delete_backfacing_triangles(
    std::list<FixedArray<ColoredVertex, 3>>* deleted_triangles)
{
    std::erase_if(triangles_, [deleted_triangles](const FixedArray<ColoredVertex, 3>& t) -> bool{
        bool erase = dot0d(scaled_triangle_normal({
                t(0).position,
                t(1).position,
                t(2).position}),
            FixedArray<float, 3>{0.f, 0.f, 1.f}) <= 0;
        if (erase) {
            if (deleted_triangles != nullptr) {
                deleted_triangles->push_back(t);
            }
            // std::cerr << "Triangle at has negative normal direction" << std::endl;
            // draw_node(*triangles, {t(0).position(0), t(0).position(1)}, scale * 5);
            // draw_node(*triangles, {t(1).position(0), t(1).position(1)}, scale * 5);
            // draw_node(*triangles, {t(2).position(0), t(2).position(1)}, scale * 5);
        }
        return erase;
    });
}

void TriangleList::calculate_triangle_normals() {
    for (auto& t : triangles_) {
        auto n = triangle_normal({t(0).position, t(1).position, t(2).position});
        t(0).normal = n;
        t(1).normal = n;
        t(2).normal = n;
    }
}

void TriangleList::convert_triangle_to_vertex_normals() {
    VertexNormals vertex_normals;
    vertex_normals.add_triangles(triangles_.begin(), triangles_.end());
    for (auto& it : triangles_) {
        for (auto& v : it.flat_iterable()) {
            v.normal = vertex_normals.get_normal(v.position);
        }
    }
}

void TriangleList::flip() {
    for (auto& l : triangles_) {
        for (auto& v : l.flat_iterable()) {
            v.normal = -v.normal;
        }
        std::swap(l(0), l(1));
    }
}

void TriangleList::convert_triangle_to_vertex_normals(const std::list<std::shared_ptr<TriangleList>>& triangle_lists) {
    VertexNormals vertex_normals;
    for (const auto& l : triangle_lists) {
        vertex_normals.add_triangles(
            l->triangles_.begin(),
            l->triangles_.end());
    }
    vertex_normals.compute_vertex_normals();
    for (const auto& l : triangle_lists) {
        for (auto& it : l->triangles_) {
            for (auto& v : it.flat_iterable()) {
                v.normal = vertex_normals.get_normal(v.position);
            }
        }
    }
}

void TriangleList::smoothen_edges(
    const std::list<std::shared_ptr<TriangleList>>& edge_triangle_lists,
    const std::list<std::shared_ptr<TriangleList>>& excluded_triangle_lists,
    const std::list<FixedArray<float, 3>*>& smoothed_vertices,
    float smoothness,
    size_t niterations,
    bool move_only_z,
    float decay)
{
    typedef OrderableFixedArray<float, 2> Vertex2;
    std::set<Vertex2> excluded_vertices;
    for (const auto& tl : excluded_triangle_lists) {
        for (const auto& t : tl->triangles_) {
            for (const auto& v : t.flat_iterable()) {
                excluded_vertices.insert(Vertex2{v.position(0), v.position(1)});
            }
        }
    }
    for (size_t i = 0; i < niterations; ++i) {
        typedef OrderableFixedArray<float, 3> Vertex3;
        typedef OrderableFixedArray<Vertex3, 2> Edge3;
        std::map<Edge3, Vertex3> edge_neighbors;
        std::map<Vertex2, FixedArray<float, 3>> vertex_movement;
        for (const auto& l : edge_triangle_lists) {
            for (const auto& t : l->triangles_) {
                auto insert_edge = [&](size_t i, size_t j, size_t n){
                    Vertex3 ei{t(i).position};
                    Vertex3 ej{t(j).position};
                    Vertex3 nn{t(n).position};
                    auto it = edge_neighbors.find(Edge3{ej, ei});
                    if (it == edge_neighbors.end()) {
                        edge_neighbors.insert({Edge3{ei, ej}, nn});
                    } else {
                        FixedArray<float, 3> n0 = triangle_normal({ej, ei, it->second});
                        FixedArray<float, 3> n1 = triangle_normal({ei, ej, nn});
                        FixedArray<float, 3> cn = (it->second + nn) / 2.f;
                        FixedArray<float, 3> ce = (ei + ej) / 2.f;
                        FixedArray<float, 3> v = cn - ce;
                        FixedArray<float, 3> n01 = (n0 + n1) / 2.f;
                        n01 /= std::sqrt(sum(squared(n01)));
                        float n0n1 = dot0d(n0, n1);
                        if (n0n1 >=0 && n0n1 < 1) {
                            float shift = std::sqrt(1 - squared(n0n1)) * sign(dot0d(v, n01));
                            if (!excluded_vertices.contains(Vertex2{ei(0), ei(1)})) {
                                vertex_movement[Vertex2{ei(0), ei(1)}] += smoothness * n01 * shift;
                            }
                            if (!excluded_vertices.contains(Vertex2{ej(0), ej(1)})) {
                                vertex_movement[Vertex2{ej(0), ej(1)}] += smoothness * n01 * shift;
                            }
                        }
                    }
                };
                insert_edge(0, 1, 2);
                insert_edge(1, 2, 0);
                insert_edge(2, 0, 1);
            }
        }
        for (const auto& s : smoothed_vertices) {
            auto it = vertex_movement.find(Vertex2{(*s)(0), (*s)(1)});
            if (it != vertex_movement.end()) {
                if (move_only_z) {
                    (*s)(2) += it->second(2);
                } else {
                    *s += it->second;
                }
            }
        }
        smoothness *= decay;
    }
}

std::shared_ptr<ColoredVertexArray> TriangleList::triangle_array() const {
    return std::make_shared<ColoredVertexArray>(
        name_,
        material_,
        std::move(std::vector<FixedArray<ColoredVertex, 3>>{triangles_.begin(), triangles_.end()}),
        std::move(std::vector<FixedArray<ColoredVertex, 2>>()),
        std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>{triangle_bone_weights_.begin(), triangle_bone_weights_.end()}),
        std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>()));
}
