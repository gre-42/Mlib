#include "Triangle_List.hpp"
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
    const FixedArray<float, 2>& u01)
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
    const FixedArray<float, 2>& u01)
{
    auto n = triangle_normal({p00, p10, p01});
    draw_triangle_with_normals(p00, p10, p01, n, n, n, c00, c10, c01, u00, u10, u01);
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
    const FixedArray<float, 2>& u01)
{
    draw_triangle_with_normals(p00, p11, p01, n00, n11, n01, c00, c11, c01, u00, u11, u01);
    draw_triangle_with_normals(p00, p10, p11, n00, n10, n11, c00, c10, c11, u00, u10, u11);
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
    const FixedArray<float, 2>& u01)
{
    draw_triangle_wo_normals(p00, p11, p01, c00, c11, c01, u00, u11, u01);
    draw_triangle_wo_normals(p00, p10, p11, c00, c10, c11, u00, u10, u11);
}

void TriangleList::extrude(float height, float scale, float uv_scale) {
    using O = OrderableFixedArray<float, 3>;

    std::set<std::pair<O, O>> edges = find_contour_edges(triangles_);
    size_t i = 0;
    size_t size0 = triangles_.size();
    for(auto& t : triangles_) {
        if (i == size0) {
            break;
        }
        FixedArray<ColoredVertex, 3> t_old = t;
        auto connect_extruded = [this, &scale, &uv_scale, &t_old, &height, &edges, &t](size_t a, size_t b){
            auto edge = std::make_pair(O(t_old(a).position), O(t_old(b).position));
            if (edges.contains(edge)) {
                draw_rectangle_wo_normals(
                    t_old(a).position,
                    t_old(b).position,
                    t(b).position,
                    t(a).position,
                    {1, 1, 1},
                    {1, 1, 1},
                    {1, 1, 1},
                    {1, 1, 1},
                    t_old(a).uv,
                    t_old(b).uv,
                    t_old(b).uv + FixedArray<float, 2>{height / scale * uv_scale, 0},
                    t_old(a).uv + FixedArray<float, 2>{height / scale * uv_scale, 0});
            }
        };
        t(0).position(2) += height;
        t(1).position(2) += height;
        t(2).position(2) += height;
        t(0).uv(0) *= uv_scale;
        t(1).uv(0) *= uv_scale;
        t(2).uv(0) *= uv_scale;

        connect_extruded(0, 1);
        connect_extruded(1, 2);
        connect_extruded(2, 0);
        ++i;
    }
}

void TriangleList::delete_backfacing_triangles() {
    std::erase_if(triangles_, [](const FixedArray<ColoredVertex, 3>& t) -> bool{
        bool erase = dot0d(scaled_triangle_normal({
                t(0).position,
                t(1).position,
                t(2).position}),
            FixedArray<float, 3>{0, 0, 1}) <= 0;
        if (erase) {
            // std::cerr << "Triangle at has negative normal direction" << std::endl;
            // draw_node(*triangles, {t(0).position(0), t(0).position(1)}, scale * 5);
            // draw_node(*triangles, {t(1).position(0), t(1).position(1)}, scale * 5);
            // draw_node(*triangles, {t(2).position(0), t(2).position(1)}, scale * 5);
        }
        return erase;
    });
}

void TriangleList::calculate_triangle_normals() {
    for(auto& t : triangles_) {
        auto n = triangle_normal({t(0).position, t(1).position, t(2).position});
        t(0).normal = n;
        t(1).normal = n;
        t(2).normal = n;
    }
}

void TriangleList::convert_triangle_to_vertex_normals(std::list<std::shared_ptr<TriangleList>>& triangle_lists) {
    VertexNormals vertex_normals;
    for(const auto& l : triangle_lists) {
        vertex_normals.add_triangles(
            l->triangles_.begin(),
            l->triangles_.end());
    }
    vertex_normals.compute_vertex_normals();
    for(const auto& l : triangle_lists) {
        for(auto& it : l->triangles_) {
            for(auto& v : it.flat_iterable()) {
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
    size_t niterations)
{
    std::set<OrderableFixedArray<float, 3>> excluded_vertices;
    for(const auto& tl : excluded_triangle_lists) {
        for(const auto& t : tl->triangles_) {
            for(const auto& v : t.flat_iterable()) {
                excluded_vertices.insert(OrderableFixedArray{v.position});
            }
        }
    }
    for(size_t i = 0; i < niterations; ++i) {
        typedef OrderableFixedArray<float, 2> Vertex2;
        typedef OrderableFixedArray<float, 3> Vertex3;
        typedef OrderableFixedArray<Vertex3, 2> Edge3;
        std::map<Edge3, Vertex3> edge_neighbors;
        std::map<Vertex2, FixedArray<float, 3>> vertex_movement;
        for(const auto& l : edge_triangle_lists) {
            for(const auto& t : l->triangles_) {
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
                            if (!excluded_vertices.contains(ei)) {
                                vertex_movement[Vertex2{ei(0), ei(1)}] += smoothness * 0.0001f * n01 * shift;
                            }
                            if (!excluded_vertices.contains(ej)) {
                                vertex_movement[Vertex2{ej(0), ej(1)}] += smoothness * 0.0001f * n01 * shift;
                            }
                        }
                    }
                };
                insert_edge(0, 1, 2);
                insert_edge(1, 2, 0);
                insert_edge(2, 0, 1);
            }
        }
        for(const auto& s : smoothed_vertices) {
            auto it = vertex_movement.find(Vertex2{(*s)(0), (*s)(1)});
            if (it != vertex_movement.end()) {
                *s += it->second;
            }
        }
    }
}

std::list<FixedArray<ColoredVertex, 3>> TriangleList::get_triangles_around(const FixedArray<float, 2>& pt, float radius) const {
    std::list<FixedArray<ColoredVertex, 3>> tf;
    for(const auto& t : triangles_) {
        FixedArray<float, 2> a{t(0).position(0), t(0).position(1)};
        FixedArray<float, 2> b{t(1).position(0), t(1).position(1)};
        FixedArray<float, 2> c{t(2).position(0), t(2).position(1)};
        if ((sum(squared(a - pt)) < squared(radius)) ||
            (sum(squared(b - pt)) < squared(radius)) ||
            (sum(squared(c - pt)) < squared(radius)))
        {
            tf.push_back(t);
        }
    }
    return tf;
}

std::shared_ptr<ColoredVertexArray> TriangleList::triangle_array() const {
    return std::make_shared<ColoredVertexArray>(
        name_,
        material_,
        std::move(std::vector<FixedArray<ColoredVertex, 3>>{triangles_.begin(), triangles_.end()}),
        std::move(std::vector<FixedArray<ColoredVertex, 2>>()));
}
