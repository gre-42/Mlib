#include "Triangle_List.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Static_Face_Lightning.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
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
void TriangleList::delete_backfacing_triangles() {
    for(auto it = triangles_.begin(); it != triangles_.end(); ) {
        auto it0 = it++;
        const auto& t = *it0;
        if (dot(scaled_triangle_normal({
                t(0).position,
                t(1).position,
                t(2).position}),
            FixedArray<float, 3>{0, 0, 1})() <= 0)
        {
            // std::cerr << "Triangle at has negative normal direction" << std::endl;
            triangles_.erase(it0);
            // draw_node(*triangles, {t(0).position(0), t(0).position(1)}, scale * 5);
            // draw_node(*triangles, {t(1).position(0), t(1).position(1)}, scale * 5);
            // draw_node(*triangles, {t(2).position(0), t(2).position(1)}, scale * 5);
        }
    }
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
    for(auto l : triangle_lists) {
        vertex_normals.add_triangles(
            l->triangles_.begin(),
            l->triangles_.end());
    }
    vertex_normals.compute_vertex_normals();
    for(auto l : triangle_lists) {
        for(auto& it : l->triangles_) {
            for(auto& v : it.flat_iterable()) {
                v.normal = vertex_normals.get_normal(v.position);
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
