#include "Colored_Vertex_Array.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Delaunay.hpp>
#include <Mlib/Geometry/Delaunay_Error_Behavior.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Line_3D.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Polygon_3D.hpp>
#include <Mlib/Geometry/Quad_3D.hpp>
#include <Mlib/Geometry/Triangle_3D.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <mutex>
#include <set>

using namespace Mlib;

template <class TPos>
ColoredVertexArray<TPos>::ColoredVertexArray(
    GroupAndName name,
    const Material& material,
    const Morphology& morphology,
    ModifierBacklog modifier_backlog,
    UUVector<FixedArray<ColoredVertex<TPos>, 4>>&& quads,
    UUVector<FixedArray<ColoredVertex<TPos>, 3>>&& triangles,
    UUVector<FixedArray<ColoredVertex<TPos>, 2>>&& lines,
    UUVector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
    UUVector<FixedArray<float, 3>>&& continuous_triangle_texture_layers,
    UUVector<FixedArray<uint8_t, 3>>&& discrete_triangle_texture_layers,
    std::vector<UUVector<FixedArray<float, 3, 2>>>&& uv1,
    std::vector<UUVector<FixedArray<float, 3>>>&& cweight,
    UUVector<FixedArray<float, 3>>&& alpha,
    UUVector<FixedArray<float, 4>>&& interiormap_uvmaps,
    const ExtremalAxisAlignedBoundingBox<TPos, 3>* aabb,
    const ExtremalBoundingSphere<TPos, 3>* bounding_sphere)
    : name{ std::move(name) }
    , material{ material }
    , morphology{ morphology }
    , modifier_backlog{ modifier_backlog }
    , quads{ std::move(quads) }
    , triangles{ std::move(triangles) }
    , lines{ std::move(lines) }
    , triangle_bone_weights{ std::move(triangle_bone_weights) }
    , continuous_triangle_texture_layers{ std::move(continuous_triangle_texture_layers) }
    , discrete_triangle_texture_layers{ std::move(discrete_triangle_texture_layers) }
    , uv1{ std::move(uv1) }
    , cweight{ std::move(cweight) }
    , alpha{ std::move(alpha) }
    , interiormap_uvmaps{ std::move(interiormap_uvmaps) }
    , aabb_has_value_{ false }
    , bounding_sphere_has_value_{ false }
{
    assert_true(!this->name.empty());
    if (!this->triangle_bone_weights.empty() && (this->triangle_bone_weights.size() != this->triangles.size())) {
        THROW_OR_ABORT("Triangle bone weights size mismatch");
    }
    if (!this->continuous_triangle_texture_layers.empty() && (this->continuous_triangle_texture_layers.size() != this->triangles.size())) {
        THROW_OR_ABORT("Continuous triangle texture layers size mismatch");
    }
    if (!this->discrete_triangle_texture_layers.empty() && (this->discrete_triangle_texture_layers.size() != this->triangles.size())) {
        THROW_OR_ABORT("Discrete triangle texture layers size mismatch");
    }
    for (const auto& u : uv1) {
        if (u.size() != this->triangles.size()) {
            THROW_OR_ABORT("UV1 size mismatch");
        }
    }
    for (const auto& b : cweight) {
        if (b.size() != this->triangles.size()) {
            THROW_OR_ABORT("cweight size mismatch");
        }
    }
    if (!alpha.empty() && (alpha.size() != this->triangles.size())) {
        THROW_OR_ABORT("alpha size mismatch");
    }
    if ((aabb == nullptr) != (bounding_sphere == nullptr)) {
        THROW_OR_ABORT("Inconsistent AABB/bounding sphere arguments");
    }
    if (aabb != nullptr) {
        aabb_ = *aabb;
        aabb_has_value_ = true;
    }
    if (bounding_sphere != nullptr) {
        bounding_sphere_ = *bounding_sphere;
        bounding_sphere_has_value_ = true;
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
bool ColoredVertexArray<TPos>::empty() const {
    return empty_from<PrimitiveDimensions::BEGIN>();
}

template <class TPos>
template <PrimitiveDimensions tfirst_dim>
bool ColoredVertexArray<TPos>::empty_from() const {
    if constexpr (tfirst_dim == PrimitiveDimensions::END) {
        return true;
    } else {
        return primitives<tfirst_dim>().empty() && empty_from<tfirst_dim + 1>();
    }
}

template <class TPos>
UUVector<FixedArray<TPos, 3>> ColoredVertexArray<TPos>::vertices() const {
    UUVector<FixedArray<TPos, 3>> res;
    res.reserve(quads.size() * 4 + triangles.size() * 3 + lines.size() * 2);
    for (const auto& v : quads) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
        res.push_back(v(2).position);
        res.push_back(v(3).position);
    }
    for (const auto& v : triangles) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
        res.push_back(v(2).position);
    }
    for (const auto& v : lines) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
    }
    return res;
}

// FixedArray<float, 4, 4> weighted_bones_transformation_matrix(
//     const std::vector<BoneWeight>& weights,
//     const UUVector<FixedArray<float, 4, 4>>& m)
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
    const UUVector<OffsetAndQuaternion<float, TPosTransform>>& qs,
    const std::string& suffix) const
{
    if (!lines.empty()) {
        THROW_OR_ABORT("Cannot apply bone transformations on lines");
    }
    UUVector<FixedArray<ColoredVertex<TPosResult>, 3>> transformed_triangles;
    {
        if (triangle_bone_weights.size() != triangles.size()) {
            THROW_OR_ABORT("Size mismatch in triangle bone weights");
        }
        auto wit = triangle_bone_weights.begin();
        transformed_triangles.reserve(triangles.size());
        for (const auto& tri : triangles) {
            transformed_triangles.emplace_back(
                tri(0).transformed((*wit)(0), qs),
                tri(1).transformed((*wit)(1), qs),
                tri(2).transformed((*wit)(2), qs));
            ++wit;
        }
    }
    return std::make_shared<ColoredVertexArray<TPosResult>>(
        name + suffix,
        material,
        morphology,
        modifier_backlog,
        UUVector<FixedArray<ColoredVertex<TPosResult>, 4>>{},
        std::move(transformed_triangles),
        UUVector<FixedArray<ColoredVertex<TPosResult>, 2>>{},
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
        UUVector<FixedArray<float, 3>>(continuous_triangle_texture_layers.begin(), continuous_triangle_texture_layers.end()),
        UUVector<FixedArray<uint8_t, 3>>(discrete_triangle_texture_layers.begin(), discrete_triangle_texture_layers.end()),
        std::vector(uv1),
        std::vector(cweight),
        UUVector<FixedArray<float, 3>>(alpha),
        std::vector(interiormap_uvmaps));
}

template <class TPos>
template <class TPosResult, class TPosTransform>
std::shared_ptr<ColoredVertexArray<TPosResult>> ColoredVertexArray<TPos>::transformed(
    const TransformationMatrix<float, TPosTransform, 3>& tm,
    const std::string& suffix) const
{
    auto r = tm.R / tm.get_scale();
    UUVector<FixedArray<ColoredVertex<TPosResult>, 4>> transformed_quads;
    UUVector<FixedArray<ColoredVertex<TPosResult>, 3>> transformed_triangles;
    UUVector<FixedArray<ColoredVertex<TPosResult>, 2>> transformed_lines;
    transformed_quads.reserve(quads.size());
    for (const auto& quad : quads) {
        transformed_quads.emplace_back(
            (quad(0).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>(),
            (quad(1).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>(),
            (quad(2).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>(),
            (quad(3).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>());
    }
    transformed_triangles.reserve(triangles.size());
    for (const auto& tri : triangles) {
        transformed_triangles.emplace_back(
            (tri(0).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>(),
            (tri(1).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>(),
            (tri(2).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>());
    }
    transformed_lines.reserve(lines.size());
    for (const auto& li : lines) {
        transformed_lines.emplace_back(
            (li(0).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>(),
            (li(1).template casted<TPosTransform>()).transformed(tm, r).template casted<TPosResult>());
    }
    return std::make_shared<ColoredVertexArray<TPosResult>>(
        name + suffix,
        material,
        morphology,
        modifier_backlog,
        std::move(transformed_quads),
        std::move(transformed_triangles),
        std::move(transformed_lines),
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
        UUVector<FixedArray<float, 3>>(continuous_triangle_texture_layers.begin(), continuous_triangle_texture_layers.end()),
        UUVector<FixedArray<uint8_t, 3>>(discrete_triangle_texture_layers.begin(), discrete_triangle_texture_layers.end()),
        std::vector(uv1),
        std::vector(cweight),
        UUVector<FixedArray<float, 3>>(alpha),
        std::vector(interiormap_uvmaps));
}

template <class TPos>
template <class TPosResult, class TPosTranslation>
std::shared_ptr<ColoredVertexArray<TPosResult>> ColoredVertexArray<TPos>::translated(
    const FixedArray<TPosTranslation, 3>& t,
    const std::string& suffix) const
{
    UUVector<FixedArray<ColoredVertex<TPosResult>, 4>> transformed_quads;
    UUVector<FixedArray<ColoredVertex<TPosResult>, 3>> transformed_triangles;
    UUVector<FixedArray<ColoredVertex<TPosResult>, 2>> transformed_lines;
    transformed_quads.reserve(quads.size());
    for (const auto& quad : quads) {
        transformed_quads.emplace_back(
            (quad(0).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>(),
            (quad(1).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>(),
            (quad(2).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>(),
            (quad(3).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>());
    }
    transformed_triangles.reserve(triangles.size());
    for (const auto& tri : triangles) {
        transformed_triangles.emplace_back(
            (tri(0).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>(),
            (tri(1).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>(),
            (tri(2).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>());
    }
    transformed_lines.reserve(lines.size());
    for (const auto& li : lines) {
        transformed_lines.emplace_back(
            (li(0).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>(),
            (li(1).template casted<TPosTranslation>()).translated(t).template casted<TPosResult>());
    }
    return std::make_shared<ColoredVertexArray<TPosResult>>(
        name + suffix,
        material,
        morphology,
        modifier_backlog,
        std::move(transformed_quads),
        std::move(transformed_triangles),
        std::move(transformed_lines),
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
        UUVector<FixedArray<float, 3>>(continuous_triangle_texture_layers.begin(), continuous_triangle_texture_layers.end()),
        UUVector<FixedArray<uint8_t, 3>>(discrete_triangle_texture_layers.begin(), discrete_triangle_texture_layers.end()),
        std::vector(uv1),
        std::vector(cweight),
        UUVector<FixedArray<float, 3>>(alpha),
        std::vector(interiormap_uvmaps));
}

template <class TPos>
void ColoredVertexArray<TPos>::quads_sphere(
    std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>& collision_polygons) const
{
    polygon_sphere(collision_polygons);
}

template <class TPos>
void ColoredVertexArray<TPos>::triangles_sphere(
    std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>& collision_polygons) const
{
    polygon_sphere(collision_polygons);
}

template <class TPos>
template <class TPosTransform>
std::vector<CollisionPolygonAabb<CompressedScenePos, 4>> ColoredVertexArray<TPos>::transformed_quads_bbox(
    const TransformationMatrix<float, TPosTransform, 3>& tm) const
{
    return transformed_polygon_bbox<4, TPosTransform>(tm);
}

template <class TPos>
template <class TPosTransform>
std::vector<CollisionPolygonAabb<CompressedScenePos, 3>> ColoredVertexArray<TPos>::transformed_triangles_bbox(
    const TransformationMatrix<float, TPosTransform, 3>& tm) const
{
    return transformed_polygon_bbox<3, TPosTransform>(tm);
}

template <class TPos>
template <size_t tnvertices>
void ColoredVertexArray<TPos>::polygon_sphere(
    std::vector<CollisionPolygonSphere<CompressedScenePos, tnvertices>>& collision_polygons) const
{
    const auto& prims = primitives<(PrimitiveDimensions)tnvertices>();
    size_t len0 = collision_polygons.size();
    if (len0 + prims.size() > collision_polygons.capacity()) {
        THROW_OR_ABORT("Transformed vector has insufficient capacity");
    }
    auto rng = welzl_rng();
    for (const auto& q : prims) {
        Polygon3D<CompressedScenePos, tnvertices> poly{ q };
        collision_polygons.push_back(CollisionPolygonSphere<CompressedScenePos, tnvertices>{
            .bounding_sphere = poly.bounding_sphere(rng),
            .polygon = poly.polygon().template casted<SceneDir, CompressedScenePos>(),
            .physics_material = morphology.physics_material,
            .corners = poly.vertices()
        });
    }
    VertexNormals<TPos, float> vertex_normals;
    for (size_t i = len0; i < collision_polygons.size(); ++i) {
        const auto& poly = collision_polygons[i];
        for (const auto& v : poly.corners.row_iterable()) {
            vertex_normals.add_vertex_face_normal(
                v.template casted<TPos>(),
                poly.polygon.plane.normal.template casted<float>());
        }
    }
}

template <class TPos>
template <size_t tnvertices, class TPosTransform>
std::vector<CollisionPolygonAabb<CompressedScenePos, tnvertices>> ColoredVertexArray<TPos>::transformed_polygon_bbox(
    const TransformationMatrix<float, TPosTransform, 3>& tm) const
{
    const auto& prims = primitives<(PrimitiveDimensions)tnvertices>();
    std::vector<CollisionPolygonAabb<CompressedScenePos, tnvertices>> res;
    res.reserve(prims.size());
    auto rng = welzl_rng();
    for (const auto& q : prims) {
        Polygon3D<CompressedScenePos, tnvertices> poly{ q, tm };
        res.push_back(CollisionPolygonAabb<CompressedScenePos, tnvertices>{
            .base = CollisionPolygonSphere<CompressedScenePos, tnvertices>{
                .bounding_sphere = poly.bounding_sphere(rng),
                .polygon = poly.polygon().template casted<SceneDir, CompressedScenePos>(),
                .physics_material = morphology.physics_material,
                .corners = poly.vertices()
            },
            .aabb = poly.aabb()});
    }
    VertexNormals<ScenePos, float> vertex_normals;
    for (const auto& poly : res) {
        for (const auto& v : poly.base.corners.row_iterable()) {
            vertex_normals.add_vertex_face_normal(
                v.template casted<ScenePos>(),
                poly.base.polygon.plane.normal.template casted<float>());
        }
    }
    return res;
}

template <class TPos>
template <class TPosTransform>
std::vector<CollisionLineAabb<CompressedScenePos>> ColoredVertexArray<TPos>::transformed_lines_bbox(
    const TransformationMatrix<float, TPosTransform, 3>& tm) const
{
    std::vector<CollisionLineAabb<CompressedScenePos>> res;
    res.reserve(lines.size());
    for (const auto& l : lines) {
        Line3D<ScenePos> line{ l, tm };
        res.push_back(CollisionLineAabb{
            .base = CollisionLineSphere{
                .bounding_sphere = line.bounding_sphere().casted<CompressedScenePos>(),
                .physics_material = morphology.physics_material,
                .line = line.vertices().template casted<CompressedScenePos>(),
                .ray = line.template ray<SceneDir>().template casted<SceneDir, CompressedScenePos>()
            },
            .aabb = line.aabb().template casted<CompressedScenePos>()});
    }
    return res;
}

template <class TPos>
std::vector<CollisionLineSphere<CompressedScenePos>> ColoredVertexArray<TPos>::lines_sphere() const
{
    std::vector<CollisionLineSphere<CompressedScenePos>> res;
    res.reserve(lines.size());
    for (const auto& l : lines) {
        Line3D<ScenePos> line{ l };
        res.push_back(CollisionLineSphere{
            .bounding_sphere = line.bounding_sphere().template casted<CompressedScenePos>(),
            .physics_material = morphology.physics_material,
            .line = line.vertices().template casted<CompressedScenePos>(),
            .ray = line.template ray<SceneDir>().template casted<SceneDir, CompressedScenePos>()});
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
std::shared_ptr<ColoredVertexArray<TPos>> ColoredVertexArray<TPos>::generate_grind_lines(float edge_angle, float averaged_normal_angle) const {
    float cos_edge_angle = std::cos(edge_angle);
    float cos_averaged_normal_angle = std::cos(averaged_normal_angle);
    UUVector<FixedArray<ColoredVertex<TPos>, 2>> grind_lines;
    grind_lines.reserve(3 * triangles.size());
    using O = OrderableFixedArray<TPos, 3>;
    using Triangle = FixedArray<TPos, 3, 3>;
    using I = funpack_t<TPos>;
    std::map<std::pair<O, O>, FixedArray<I, 3>> edge_normals;
    for (const auto& t : triangles) {
        auto n = triangle_normal(funpack(Triangle{ t(0).position, t(1).position, t(2).position }));
        for (size_t i = 0; i < CW::length(t); ++i) {
            std::pair<O, O> edge0{ t(i).position, t((i + 1) % CW::length(t)).position };
            auto it = edge_normals.find(edge0);
            if (it != edge_normals.end()) {
                if (dot0d(n, it->second) > cos_edge_angle) {
                    continue;
                }
                auto m = n + it->second;
                auto l2 = sum(squared(m));
                if (l2 < 1e-12) {
                    continue;
                }
                m /= std::sqrt(l2);
                if (m(1) < cos_averaged_normal_angle) {
                    continue;
                }
                grind_lines.push_back({ t(i), t((i + 1) % CW::length(t)) });
            } else {
                std::pair<O, O> edge1{ t((i + 1) % CW::length(t)).position, t(i).position };
                edge_normals.insert({ edge1, n });
            }
        }
    }
    grind_lines.shrink_to_fit();
    return std::make_shared<ColoredVertexArray>(
        name + "_grind_lines",
        Material{},
        Morphology{ .physics_material = PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::OBJ_GRIND_LINE },
        ModifierBacklog{},
        UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
        UUVector<FixedArray<ColoredVertex<TPos>, 3>>{},
        std::move(grind_lines),
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<uint8_t, 3>>{},
        std::vector<UUVector<FixedArray<float, 3, 2>>>{},
        std::vector<UUVector<FixedArray<float, 3>>>{},
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<float, 4>>{});
}

template <class TPos>
std::shared_ptr<ColoredVertexArray<TPos>> ColoredVertexArray<TPos>::generate_contour_edges() const {
    using O = OrderableFixedArray<TPos, 3>;
    std::set<std::pair<O, O>> edges;
    for (const auto& t : triangles) {
        for (size_t i = 0; i < CW::length(t); ++i) {
            std::pair<O, O> edge0{ t(i).position, t((i + 1) % CW::length(t)).position };
            if (!edges.insert(edge0).second) {
                THROW_OR_ABORT("Could not insert edge for contour edge calculation");
            }
            std::pair<O, O> edge1{ edge0.second, edge0.first };
            edges.erase(edge1);
        }
    }
    UUVector<FixedArray<ColoredVertex<TPos>, 2>> contour_edges;
    contour_edges.reserve(edges.size());
    for (const auto& e : edges) {
        contour_edges.push_back({
            ColoredVertex<TPos>{e.first},
            ColoredVertex<TPos>{e.second}});
    }
    return std::make_shared<ColoredVertexArray>(
        name + "_contour_edges",
        Material{},
        Morphology{ .physics_material = PhysicsMaterial::ATTR_COLLIDE },
        ModifierBacklog{},
        UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
        UUVector<FixedArray<ColoredVertex<TPos>, 3>>{},
        std::move(contour_edges),
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<uint8_t, 3>>{},
        std::vector<UUVector<FixedArray<float, 3, 2>>>{},
        std::vector<UUVector<FixedArray<float, 3>>>{},
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<float, 4>>{});
}

template <class TPos>
std::shared_ptr<ColoredVertexArray<TPos>> ColoredVertexArray<TPos>::triangulate(
    RectangleTriangulationMode mode,
    DelaunayErrorBehavior error_behavior) const
{
    UUVector<FixedArray<ColoredVertex<TPos>, 3>> res_triangles;
    res_triangles.reserve(triangles.size() + 2 * quads.size());
    res_triangles.insert(res_triangles.end(), triangles.begin(), triangles.end());
    for (const auto& q : quads) {
        auto s = is_delaunay(q(0).position, q(1).position, q(2).position, q(3).position);
        if (s == DelaunayState::DELAUNAY) {
            res_triangles.emplace_back(q(0), q(2), q(3));
            res_triangles.emplace_back(q(0), q(1), q(2));
        } else if (s == DelaunayState::NOT_DELAUNAY) {
            res_triangles.emplace_back(q(0), q(1), q(3));
            res_triangles.emplace_back(q(3), q(1), q(2));
        } else if (s != DelaunayState::ERROR) {
            THROW_OR_ABORT("Unknown Delaunay state: " + std::to_string((int)s));
        } else if (error_behavior == DelaunayErrorBehavior::SKIP) {
            // Do nothing
        } else if (error_behavior == DelaunayErrorBehavior::WARN) {
            lwarn() << "Degenerate quad for triangulation";
        } else if (error_behavior == DelaunayErrorBehavior::THROW) {
            THROW_OR_ABORT("Degenerate quad for triangulation");
        } else {
            THROW_OR_ABORT("Unknown Delaunay error behavior: " + std::to_string((int)error_behavior));
        }
    }
    return std::make_shared<ColoredVertexArray<TPos>>(
        name + "_triangulated",
        material,
        morphology,
        modifier_backlog,
        UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
        std::move(res_triangles),
        UUVector<FixedArray<ColoredVertex<TPos>, 2>>{},
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<uint8_t, 3>>{},
        std::vector<UUVector<FixedArray<float, 3, 2>>>{},
        std::vector<UUVector<FixedArray<float, 3>>>{},
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<float, 4>>{});
}

template <class TPos>
std::vector<std::shared_ptr<ColoredVertexArray<TPos>>> ColoredVertexArray<TPos>::split(
    float depth,
    PhysicsMaterial destination_physics_material) const
{
    if (!any(morphology.physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
        THROW_OR_ABORT("Terrain to be decomposed is not collidable");
    }
    if (!any(destination_physics_material & PhysicsMaterial::ATTR_CONCAVE)) {
        THROW_OR_ABORT("Destination mesh is not tagged as concave");
    }

    std::vector<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    result.reserve(triangles.size() + 1);
    result.push_back(
        std::make_shared<ColoredVertexArray<TPos>>(
            name + "_visual",
            material,
            morphology - PhysicsMaterial::ATTR_COLLIDE,
            modifier_backlog,
            UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
            std::vector{triangles},
            UUVector<FixedArray<ColoredVertex<TPos>, 2>>{},
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
            UUVector<FixedArray<float, 3>>{},
            UUVector<FixedArray<uint8_t, 3>>{},
            std::vector<UUVector<FixedArray<float, 3, 2>>>{},
            std::vector<UUVector<FixedArray<float, 3>>>{},
            UUVector<FixedArray<float, 3>>{},
            UUVector<FixedArray<float, 4>>{}));
    for (const auto& tri : triangles) {
        UUVector<FixedArray<ColoredVertex<TPos>, 3>> triangle_as_list;
    
        triangle_as_list.push_back({
            ColoredVertex<TPos>{tri(0).position, Colors::PURPLE},
            ColoredVertex<TPos>{tri(1).position, Colors::PURPLE},
            ColoredVertex<TPos>{tri(2).position, Colors::PURPLE}});
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
                (morphology - removed_attributes) + destination_physics_material,
                modifier_backlog,
                UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
                std::move(triangle_as_list),
                UUVector<FixedArray<ColoredVertex<TPos>, 2>>{},
                UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
                UUVector<FixedArray<float, 3>>{},
                UUVector<FixedArray<uint8_t, 3>>{},
                std::vector<UUVector<FixedArray<float, 3, 2>>>{},
                std::vector<UUVector<FixedArray<float, 3>>>{},
                UUVector<FixedArray<float, 3>>{},
                UUVector<FixedArray<float, 4>>{}));
    }
    return result;
}

template <class TPos>
std::string ColoredVertexArray<TPos>::identifier() const {
    if (material.textures_color.size() > 0) {
        return name.full_name() + ", " + material.identifier() + ", #tris: " + std::to_string(triangles.size());
    } else {
        return name.full_name() + ", #tris: " + std::to_string(triangles.size());
    }
}

template <class TPos>
void ColoredVertexArray<TPos>::print(std::ostream& ostr) const {
    ostr << "ColoredVertexArray(" << name << "): ";
    ostr << "  visible = " << int(any(morphology.physics_material & PhysicsMaterial::ATTR_VISIBLE)) << ' ';
    ostr << "  #triangles = " << triangles.size() << ' ';
    ostr << "  #lines = " << lines.size() << ' ';
    ostr << "  #triangle_bone_weights = " << triangle_bone_weights.size() << ' ';
    ostr << "  #continuous_triangle_texture_layers = " << continuous_triangle_texture_layers.size() << ' ';
    ostr << "  #interiormap_uvmaps = " << interiormap_uvmaps.size() << ' ';
    ostr << "  #discrete_triangle_texture_layers = " << discrete_triangle_texture_layers.size() << '\n';
}

template <class TPos>
const ExtremalAxisAlignedBoundingBox<TPos, 3>& ColoredVertexArray<TPos>::aabb() const {
    if (aabb_has_value_) {
        return *aabb_;
    }
    std::scoped_lock lock{ aabb_mutex_ };
    if (aabb_.has_value()) {
        return *aabb_;
    }
    auto vs = vertices();
    if (vs.empty()) {
        THROW_OR_ABORT("Cannot compute AABB of \"" + name.full_name() + "\" because it has no vertices");
    }
    aabb_.emplace(AxisAlignedBoundingBox<TPos, 3>::from_iterator(vs.begin(), vs.end()));
    aabb_has_value_ = true;
    return *aabb_;
}

template <class TPos>
const ExtremalBoundingSphere<TPos, 3>& ColoredVertexArray<TPos>::bounding_sphere() const {
    if (bounding_sphere_has_value_) {
        return *bounding_sphere_;
    }
    std::scoped_lock lock{ bounding_sphere_mutex_ };
    if (bounding_sphere_.has_value()) {
        return *bounding_sphere_;
    }
    auto vs = vertices();
    if (vs.empty()) {
        THROW_OR_ABORT("Cannot compute bounding sphere of \"" + name.full_name() + "\" because it has no vertices");
    }
    bounding_sphere_.emplace(BoundingSphere<TPos, 3>::from_iterator(vs.begin(), vs.end()));
    bounding_sphere_has_value_ = true;
    return *bounding_sphere_;
}

template <class TPos>
void ColoredVertexArray<TPos>::set_bounds(
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    const BoundingSphere<TPos, 3>& bounding_sphere)
{
    {
        std::scoped_lock lock{ aabb_mutex_ };
        if (aabb_.has_value()) {
            THROW_OR_ABORT("AABB already set");
        }
        aabb_ = aabb;
        aabb_has_value_ = true;
    }
    {
        std::scoped_lock lock{ bounding_sphere_mutex_ };
        if (bounding_sphere_.has_value()) {
            THROW_OR_ABORT("Bounding sphere already set");
        }
        bounding_sphere_ = bounding_sphere;
        bounding_sphere_has_value_ = true;
    }
}

template <class TPos>
ScenePos ColoredVertexArray<TPos>::max_center_distance(BillboardId billboard_id) const {
    return material.max_center_distance(billboard_id, morphology, name.full_name());
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

template class Mlib::ColoredVertexArray<float>;
template class Mlib::ColoredVertexArray<CompressedScenePos>;

template std::shared_ptr<ColoredVertexArray<CompressedScenePos>> ColoredVertexArray<CompressedScenePos>::transformed(
    const TransformationMatrix<float, ScenePos, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<CompressedScenePos>> ColoredVertexArray<float>::transformed(
    const TransformationMatrix<float, ScenePos, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<float>> ColoredVertexArray<float>::transformed(
    const TransformationMatrix<float, float, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<float>> ColoredVertexArray<CompressedScenePos>::transformed(
    const TransformationMatrix<float, ScenePos, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<float>> ColoredVertexArray<float>::transformed(
    const UUVector<OffsetAndQuaternion<float, float>>& qs,
    const std::string& suffix) const;

template std::shared_ptr<ColoredVertexArray<float>> ColoredVertexArray<CompressedScenePos>::translated(
    const FixedArray<CompressedScenePos, 3>& tm,
    const std::string& suffix) const;
template std::shared_ptr<ColoredVertexArray<CompressedScenePos>> ColoredVertexArray<CompressedScenePos>::translated(
    const FixedArray<CompressedScenePos, 3>& tm,
    const std::string& suffix) const;

template std::vector<CollisionPolygonAabb<CompressedScenePos, 4>> ColoredVertexArray<CompressedScenePos>::transformed_quads_bbox(
    const TransformationMatrix<float, ScenePos, 3>& tm) const;
template std::vector<CollisionPolygonAabb<CompressedScenePos, 3>> ColoredVertexArray<CompressedScenePos>::transformed_triangles_bbox(
    const TransformationMatrix<float, ScenePos, 3>& tm) const;
template std::vector<CollisionLineAabb<CompressedScenePos>> ColoredVertexArray<CompressedScenePos>::transformed_lines_bbox(
    const TransformationMatrix<float, ScenePos, 3>& tm) const;
template std::vector<CollisionPolygonAabb<CompressedScenePos, 4>> ColoredVertexArray<float>::transformed_quads_bbox(
    const TransformationMatrix<float, ScenePos, 3>& tm) const;
template std::vector<CollisionPolygonAabb<CompressedScenePos, 3>> ColoredVertexArray<float>::transformed_triangles_bbox(
    const TransformationMatrix<float, ScenePos, 3>& tm) const;
template std::vector<CollisionLineAabb<CompressedScenePos>> ColoredVertexArray<float>::transformed_lines_bbox(
    const TransformationMatrix<float, ScenePos, 3>& tm) const;
