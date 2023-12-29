#include "Colored_Vertex_Array.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Line_3D.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Quad_3D.hpp>
#include <Mlib/Geometry/Triangle_3D.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <mutex>
#include <set>

using namespace Mlib;

template <class TPos>
ColoredVertexArray<TPos>::ColoredVertexArray(
    const std::string& name,
    Material material,
    PhysicsMaterial physics_material,
    ModifierBacklog modifier_backlog,
    std::vector<FixedArray<ColoredVertex<TPos>, 4>>&& quads,
    std::vector<FixedArray<ColoredVertex<TPos>, 3>>&& triangles,
    std::vector<FixedArray<ColoredVertex<TPos>, 2>>&& lines,
    std::vector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
    std::vector<FixedArray<std::vector<BoneWeight>, 2>>&& line_bone_weights,
    std::vector<FixedArray<uint8_t, 3>>&& triangle_texture_layers,
    std::vector<FixedArray<uint8_t, 2>>&& line_texture_layers)
: name{name},
  material{std::move(material)},
  physics_material{physics_material},
  modifier_backlog{modifier_backlog},
  quads{std::move(quads)},
  triangles{std::move(triangles)},
  lines{std::move(lines)},
  triangle_bone_weights{std::move(triangle_bone_weights)},
  line_bone_weights{std::move(line_bone_weights)},
  triangle_texture_layers{std::move(triangle_texture_layers)},
  line_texture_layers{std::move(line_texture_layers)}
{
    assert_true(!name.empty());
    if (!this->triangle_bone_weights.empty() && (this->triangle_bone_weights.size() != this->triangles.size())) {
        THROW_OR_ABORT("Triangle bone weights size mismatch");
    }
    if (!this->line_bone_weights.empty() && (this->line_bone_weights.size() != this->lines.size())) {
        THROW_OR_ABORT("Line bone weights size mismatch");
    }
    if (!this->triangle_texture_layers.empty() && (this->triangle_texture_layers.size() != this->triangles.size())) {
        THROW_OR_ABORT("Triangle texture layers size mismatch");
    }
    if (!this->line_texture_layers.empty() && (this->line_texture_layers.size() != this->lines.size())) {
        THROW_OR_ABORT("Line texture layers size mismatch");
    }
}

template <class TPos>
ColoredVertexArray<TPos>::~ColoredVertexArray()
{}

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

template <class TPos>
std::vector<FixedArray<TPos, 3>> ColoredVertexArray<TPos>::vertices() const {
    std::vector<FixedArray<TPos, 3>> res;
    res.reserve(quads.size() * 4 + triangles.size() * 3 + lines.size() * 2);
    for (auto& v : quads) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
        res.push_back(v(2).position);
        res.push_back(v(3).position);
    }
    for (auto& v : triangles) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
        res.push_back(v(2).position);
    }
    for (auto& v : lines) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
    }
    return res;
}

// FixedArray<float, 4, 4> weighted_bones_transformation_matrix(
//     const std::vector<BoneWeight>& weights,
//     const std::vector<FixedArray<float, 4, 4>>& m)
// {
//     FixedArray<float, 3, 3> R(0);
//     FixedArray<float, 3> t(0);
//     for (const BoneWeight& w : weights) {
//         R += w.weight * R3_from_4x4(m[w.bone_index]);
//         t += w.weight * t3_from_4x4(m[w.bone_index]);
//     }
//     return assemble_homogeneous_4x4(R, t);
//     // float w_max = 0;
//     // size_t w_id = SIZE_MAX;
//     // size_t i = 0;
//     // for (const BoneWeight& w : weights) {
//     //     if (w.weight > w_max) {
//     //         w_max = w.weight;
//     //         w_id = w.bone_index;
//     //     }
//     //     ++i;
//     // }
//     // return m.at(w_id);
// }

template <class TPos>
template <class TPosResult, class TPosTransform>
std::shared_ptr<ColoredVertexArray<TPosResult>> ColoredVertexArray<TPos>::transformed(
    const std::vector<OffsetAndQuaternion<float, TPosTransform>>& qs,
    const std::string& suffix) const
{
    std::vector<FixedArray<ColoredVertex<TPosResult>, 3>> transformed_triangles;
    std::vector<FixedArray<ColoredVertex<TPosResult>, 2>> transformed_lines;
    {
        if (triangle_bone_weights.size() != triangles.size()) {
            THROW_OR_ABORT("Size mismatch in triangle bone weights");
        }
        auto wit = triangle_bone_weights.begin();
        transformed_triangles.reserve(triangles.size());
        for (const auto& tri : triangles) {
            transformed_triangles.push_back({
                tri(0).transformed((*wit)(0), qs),
                tri(1).transformed((*wit)(1), qs),
                tri(2).transformed((*wit)(2), qs)});
            ++wit;
        }
    }
    {
        if (line_bone_weights.size() != lines.size()) {
            THROW_OR_ABORT("Size mismatch in line bone weights");
        }
        auto wit = line_bone_weights.begin();
        transformed_lines.reserve(lines.size());
        for (const auto& li : lines) {
            transformed_lines.push_back({
                li(0).transformed((*wit)(0), qs),
                li(1).transformed((*wit)(1), qs)});
            // transformed_lines.back()(0).normalize();
            // transformed_lines.back()(1).normalize();
            ++wit;
        }
    }
    return std::make_shared<ColoredVertexArray<TPosResult>>(
        name + suffix,
        material,
        physics_material,
        modifier_backlog,
        std::vector<FixedArray<ColoredVertex<TPosResult>, 4>>{},
        std::move(transformed_triangles),
        std::move(transformed_lines),
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
        std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},
        std::vector<FixedArray<uint8_t, 3>>(triangle_texture_layers.begin(), triangle_texture_layers.end()),
        std::vector<FixedArray<uint8_t, 2>>{});
}

template <class TPos>
template <class TPosResult, class TPosTransform>
std::shared_ptr<ColoredVertexArray<TPosResult>> ColoredVertexArray<TPos>::transformed(
    const TransformationMatrix<float, TPosTransform, 3>& tm,
    const std::string& suffix) const
{
    auto r = tm.R() / tm.get_scale();
    std::vector<FixedArray<ColoredVertex<TPosResult>, 3>> transformed_triangles;
    std::vector<FixedArray<ColoredVertex<TPosResult>, 2>> transformed_lines;
    transformed_triangles.reserve(triangles.size());
    for (const auto& tri : triangles) {
        transformed_triangles.push_back({
            (tri(0) TEMPLATEV casted<TPosTransform>()).transformed(tm, r) TEMPLATEV casted<TPosResult>(),
            (tri(1) TEMPLATEV casted<TPosTransform>()).transformed(tm, r) TEMPLATEV casted<TPosResult>(),
            (tri(2) TEMPLATEV casted<TPosTransform>()).transformed(tm, r) TEMPLATEV casted<TPosResult>()});
    }
    transformed_lines.reserve(lines.size());
    for (const auto& li : lines) {
        transformed_lines.push_back({
            (li(0) TEMPLATEV casted<TPosTransform>()).transformed(tm, r) TEMPLATEV casted<TPosResult>(),
            (li(1) TEMPLATEV casted<TPosTransform>()).transformed(tm, r) TEMPLATEV casted<TPosResult>()});
    }
    return std::make_shared<ColoredVertexArray<TPosResult>>(
        name + suffix,
        material,
        physics_material,
        modifier_backlog,
        std::vector<FixedArray<ColoredVertex<TPosResult>, 4>>{},
        std::move(transformed_triangles),
        std::move(transformed_lines),
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
        std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},
        std::vector<FixedArray<uint8_t, 3>>(triangle_texture_layers.begin(), triangle_texture_layers.end()),
        std::vector<FixedArray<uint8_t, 2>>{});
}

template <class TPos>
void ColoredVertexArray<TPos>::transformed_quads_sphere(
    std::vector<CollisionPolygonSphere<4>>& transformed,
    const TransformationMatrix<float, double, 3>& tm) const
{
    if (transformed.size() + quads.size() > transformed.capacity()) {
        THROW_OR_ABORT("Transformed vector has insufficient capacity");
    }
    auto rng = welzl_rng();
    for (const auto& q : quads) {
        Quad3D quad{q, tm};
        transformed.push_back(CollisionPolygonSphere<4>{
            .bounding_sphere = quad.bounding_sphere(rng),
            .polygon = quad.polygon(),
            .physics_material = physics_material,
            .corners = quad.vertices()});
    }
}

template <class TPos>
void ColoredVertexArray<TPos>::transformed_triangles_sphere(
    std::vector<CollisionPolygonSphere<3>>& transformed,
    const TransformationMatrix<float, double, 3>& tm) const
{
    if (transformed.size() + triangles.size() > transformed.capacity()) {
        THROW_OR_ABORT("Transformed vector has insufficient capacity");
    }
    auto rng = welzl_rng();
    for (const auto& t : triangles) {
        Triangle3D tri{t, tm};
        transformed.push_back(CollisionPolygonSphere<3>{
            .bounding_sphere = tri.bounding_sphere(rng),
            .polygon = tri.polygon(),
            .physics_material = physics_material,
            .corners = tri.vertices()});
    }
}

template <class TPos>
std::vector<CollisionPolygonAabb<3>> ColoredVertexArray<TPos>::transformed_triangles_bbox(
    const TransformationMatrix<float, double, 3>& tm) const
{
    std::vector<CollisionPolygonAabb<3>> res;
    res.reserve(triangles.size());
    auto rng = welzl_rng();
    for (const auto& t : triangles) {
        Triangle3D tri{t, tm};
        res.push_back(CollisionPolygonAabb<3>{
            .base = CollisionPolygonSphere<3>{
                .bounding_sphere = tri.bounding_sphere(rng),
                .polygon = tri.polygon(),
                .physics_material = physics_material,
                .corners = tri.vertices()
            },
            .aabb = tri.aabb()});
    }
    return res;
}

template <class TPos>
std::vector<CollisionLineAabb> ColoredVertexArray<TPos>::transformed_lines_bbox(
    const TransformationMatrix<float, double, 3>& tm) const
{
    std::vector<CollisionLineAabb> res;
    res.reserve(lines.size());
    for (const auto& l : lines) {
        Line3D line{l, tm};
        res.push_back(CollisionLineAabb{
            .base = CollisionLineSphere{
                .bounding_sphere = line.bounding_sphere(),
                .line = line.vertices(),
                .ray{line.vertices()}
            },
            .aabb = line.aabb()});
    }
    return res;
}

template <class TPos>
std::vector<CollisionLineSphere> ColoredVertexArray<TPos>::transformed_lines_sphere(
    const TransformationMatrix<float, double, 3>& tm) const
{
    std::vector<CollisionLineSphere> res;
    res.reserve(lines.size());
    for (const auto& l : lines) {
        Line3D line{l, tm};
        res.push_back(CollisionLineSphere{
            .bounding_sphere = line.bounding_sphere(),
            .line = line.vertices(),
            .ray{line.vertices()} });
    }
    return res;
}

template <class TData>
std::vector<TData> downsampled_array(const std::vector<TData>& v, size_t n) {
    std::vector<TData> result;
    if (v.empty()) {
        return result;
    }
    result.reserve((v.size() - 1) / n + 1);
    for (size_t i = 0; i < v.size(); i += n) {
        result.push_back(v[i]);
    }
    assert_true(result.size() == ((v.size() - 1) / n + 1));
    return result;
}

template <class TPos>
void ColoredVertexArray<TPos>::downsample_triangles(size_t n) {
    if (n == 0) {
        THROW_OR_ABORT("Cannot downsaple by a factor of 0");
    }
    if (n == 1) {
        return;
    }
    assert_true(triangle_bone_weights.empty() || (triangles.size() == triangle_bone_weights.size()));
    triangles = downsampled_array(triangles, n);
    triangle_bone_weights = downsampled_array(triangle_bone_weights, n);
}

template <class TPos>
ColoredVertexArray<TPos> ColoredVertexArray<TPos>::generate_grind_lines(TPos edge_angle, TPos averaged_normal_angle) const {
    TPos cos_edge_angle = std::cos(edge_angle);
    TPos cos_averaged_normal_angle = std::cos(averaged_normal_angle);
    std::vector<FixedArray<ColoredVertex<TPos>, 2>> grind_lines;
    grind_lines.reserve(3 * triangles.size());
    using O = OrderableFixedArray<TPos, 3>;
    std::map<std::pair<O, O>, FixedArray<TPos, 3>> edge_normals;
    for (const auto& t : triangles) {
        auto n = triangle_normal<TPos>({ t(0).position, t(1).position, t(2).position });
        for (size_t i = 0; i < t.length(); ++i) {
            std::pair<O, O> edge0{ t(i).position, t((i + 1) % t.length()).position };
            auto it = edge_normals.find(edge0);
            if (it != edge_normals.end()) {
                if (dot0d(n, it->second) > cos_edge_angle) {
                    continue;
                }
                auto m = n + it->second;
                TPos l2 = sum(squared(m));
                if (l2 < 1e-12) {
                    continue;
                }
                m /= std::sqrt(l2);
                if (m(1) < cos_averaged_normal_angle) {
                    continue;
                }
                grind_lines.push_back({ t(i), t((i + 1) % t.length()) });
            } else {
                std::pair<O, O> edge1{ t((i + 1) % t.length()).position, t(i).position };
                edge_normals.insert({ edge1, n });
            }
        }
    }
    grind_lines.shrink_to_fit();
    return ColoredVertexArray(
        name + "_grind_lines",
        Material(),
        PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_GRIND_LINE,
        ModifierBacklog{},
        {},
        {},
        std::move(grind_lines),
        {},
        {},
        {},
        {});
}

template <class TPos>
ColoredVertexArray<TPos> ColoredVertexArray<TPos>::generate_contour_edges() const {
    using O = OrderableFixedArray<TPos, 3>;
    std::set<std::pair<O, O>> edges;
    for (const auto& t : triangles) {
        for (size_t i = 0; i < t.length(); ++i) {
            std::pair<O, O> edge0{ t(i).position, t((i + 1) % t.length()).position };
            if (!edges.insert(edge0).second) {
                THROW_OR_ABORT("Could not insert edge for contour edge calculation");
            }
            std::pair<O, O> edge1{ edge0.second, edge0.first };
            edges.erase(edge1);
        }
    }
    std::vector<FixedArray<ColoredVertex<TPos>, 2>> contour_edges;
    contour_edges.reserve(edges.size());
    for (const auto& e : edges) {
        contour_edges.push_back({
            ColoredVertex<TPos>{.position = e.first},
            ColoredVertex<TPos>{.position = e.second}});
    }
    return ColoredVertexArray(
        name + "_contour_edges",
        Material(),
        PhysicsMaterial::ATTR_COLLIDE,
        ModifierBacklog{},
        {},
        {},
        std::move(contour_edges),
        {},
        {},
        {},
        {});
}

template <class TPos>
std::vector<std::shared_ptr<ColoredVertexArray<TPos>>> ColoredVertexArray<TPos>::split(
    float depth,
    PhysicsMaterial destination_physics_material) const
{
    if (!any(physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
        THROW_OR_ABORT("Terrain to be decomposed is not collidable");
    }
    if (!any(destination_physics_material & PhysicsMaterial::ATTR_CONCAVE)) {
        THROW_OR_ABORT("Destination mesh is not tagged as concave");
    }
    VertexNormals<TPos, float> vertex_normals;
    vertex_normals.add_triangles(triangles.begin(), triangles.end());
    vertex_normals.compute_vertex_normals();

    std::vector<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    result.reserve(triangles.size() + 1);
    result.push_back(
        std::make_shared<ColoredVertexArray<TPos>>(
            name + "_visual",
            material,
            physics_material & ~PhysicsMaterial::ATTR_COLLIDE,
            modifier_backlog,
            std::vector<FixedArray<ColoredVertex<TPos>, 4>>{},
            std::vector{triangles},
            std::vector<FixedArray<ColoredVertex<TPos>, 2>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},
            std::vector<FixedArray<uint8_t, 3>>{},
            std::vector<FixedArray<uint8_t, 2>>{}));
    for (const auto& tri : triangles) {
        std::vector<FixedArray<ColoredVertex<TPos>, 3>> triangle_as_list;
    
        const auto purple = FixedArray<float, 3>{1.f, 0.f, 1.f};
        const auto zeros2 = fixed_zeros<float, 2>();
        const auto zeros3 = fixed_zeros<float, 3>();
        triangle_as_list.push_back({
            ColoredVertex<TPos>{.position = tri(0).position, .color = purple, .uv = zeros2, .normal = zeros3, .tangent = zeros3},
            ColoredVertex<TPos>{.position = tri(1).position, .color = purple, .uv = zeros2, .normal = zeros3, .tangent = zeros3},
            ColoredVertex<TPos>{.position = tri(2).position, .color = purple, .uv = zeros2, .normal = zeros3, .tangent = zeros3}});
        auto removed_attributes =
            PhysicsMaterial::ATTR_VISIBLE |
            PhysicsMaterial::ATTR_COLLIDE |
            PhysicsMaterial::ATTR_TWO_SIDED |
            PhysicsMaterial::ATTR_CONVEX |
            PhysicsMaterial::ATTR_CONCAVE;
        result.push_back(
            std::make_shared<ColoredVertexArray<TPos>>(
                name + "_triangle",
                Material{
                    .aggregate_mode = AggregateMode::ONCE
                },
                destination_physics_material | (physics_material & ~removed_attributes),
                modifier_backlog,
                std::vector<FixedArray<ColoredVertex<TPos>, 4>>{},
                std::move(triangle_as_list),
                std::vector<FixedArray<ColoredVertex<TPos>, 2>>{},
                std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
                std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},
                std::vector<FixedArray<uint8_t, 3>>{},
                std::vector<FixedArray<uint8_t, 2>>{}));
    }
    return result;
}

template <class TPos>
std::string ColoredVertexArray<TPos>::identifier() const {
    if (material.textures_color.size() > 0) {
        return name + ", " + material.identifier() + ", #tris: " + std::to_string(triangles.size());
    } else {
        return name + ", #tris: " + std::to_string(triangles.size());
    }
}

template <class TPos>
void ColoredVertexArray<TPos>::print(std::ostream& ostr) const {
    ostr << "ColoredVertexArray(" << name << "): ";
    ostr << "  visible = " << int(physics_material & PhysicsMaterial::ATTR_VISIBLE) << ' ';
    ostr << "  #triangles = " << triangles.size() << ' ';
    ostr << "  #lines = " << lines.size() << ' ';
    ostr << "  #triangle_bone_weights = " << triangle_bone_weights.size() << ' ';
    ostr << "  #line_bone_weights = " << line_bone_weights.size() << '\n';
    ostr << "  #triangle_texture_layers = " << triangle_texture_layers.size() << ' ';
    ostr << "  #line_texture_layers = " << line_texture_layers.size() << '\n';
}

template <class TPos>
AxisAlignedBoundingBox<TPos, 3> ColoredVertexArray<TPos>::aabb() const {
    {
        std::shared_lock lock{aabb_mutex_.value};
        if (aabb_.has_value()) {
            return aabb_.value();
        }
    }
    std::scoped_lock lock{aabb_mutex_.value};
    if (aabb_.has_value()) {
        return aabb_.value();
    }
    auto vs = vertices();
    if (vs.empty()) {
        THROW_OR_ABORT("Cannot compute AABB of \"" + name + "\" because it has no vertices");
    }
    aabb_ = AxisAlignedBoundingBox<TPos, 3>();
    for (const auto& v : vs) {
        aabb_.value().extend(v);
    }
    return aabb_.value();
}

template <class TPos>
double ColoredVertexArray<TPos>::max_center_distance(uint32_t billboard_id) const {
    return material.max_center_distance(billboard_id);
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

template class Mlib::ColoredVertexArray<float>;
template class Mlib::ColoredVertexArray<double>;

template std::shared_ptr<ColoredVertexArray<double>> ColoredVertexArray<double>::transformed(
    const TransformationMatrix<float, double, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<double>> ColoredVertexArray<float>::transformed(
    const TransformationMatrix<float, double, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<float>> ColoredVertexArray<float>::transformed(
    const TransformationMatrix<float, float, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<float>> ColoredVertexArray<double>::transformed(
    const TransformationMatrix<float, double, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<float>> ColoredVertexArray<float>::transformed(
    const std::vector<OffsetAndQuaternion<float, float>>& qs,
    const std::string& suffix) const;
