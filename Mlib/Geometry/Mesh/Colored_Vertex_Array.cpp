#include "Colored_Vertex_Array.hpp"
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;

ColoredVertexArray::ColoredVertexArray(
    const std::string& name,
    const Material& material,
    std::vector<FixedArray<ColoredVertex, 3>>&& triangles,
    std::vector<FixedArray<ColoredVertex, 2>>&& lines,
    std::vector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
    std::vector<FixedArray<std::vector<BoneWeight>, 2>>&& line_bone_weights)
: name{name},
  material{material},
  triangles{std::forward<std::vector<FixedArray<ColoredVertex, 3>>>(triangles)},
  lines{std::forward<std::vector<FixedArray<ColoredVertex, 2>>>(lines)},
  triangle_bone_weights{std::forward<std::vector<FixedArray<std::vector<BoneWeight>, 3>>>(triangle_bone_weights)},
  line_bone_weights{std::forward<std::vector<FixedArray<std::vector<BoneWeight>, 2>>>(line_bone_weights)}
{
    assert_true(!name.empty());
    if (!this->triangle_bone_weights.empty() && (this->triangle_bone_weights.size() != this->triangles.size())) {
        throw std::runtime_error("Triangle bone weights size mismatch");
    }
    if (!this->line_bone_weights.empty() && (this->line_bone_weights.size() != this->lines.size())) {
        throw std::runtime_error("Line bone weights size mismatch");
    }
}

#pragma GCC push_options
#pragma GCC optimize ("O3")

std::vector<FixedArray<float, 3>> ColoredVertexArray::vertices() const {
    std::vector<FixedArray<float, 3>> res;
    res.reserve(triangles.size() * 3 + lines.size() * 2);
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

std::shared_ptr<ColoredVertexArray> ColoredVertexArray::transformed(const std::vector<OffsetAndQuaternion<float>>& qs) const {
    auto res = std::make_shared<ColoredVertexArray>();
    res->material = material;
    {
        if (triangle_bone_weights.size() != triangles.size()) {
            throw std::runtime_error("Size mismatch in triangle bone weights");
        }
        auto wit = triangle_bone_weights.begin();
        res->triangles.reserve(triangles.size());
        for (const auto& tri : triangles) {
            res->triangles.push_back({
                tri(0).transformed((*wit)(0), qs),
                tri(1).transformed((*wit)(1), qs),
                tri(2).transformed((*wit)(2), qs)});
            ++wit;
        }
    }
    {
        if (line_bone_weights.size() != lines.size()) {
            throw std::runtime_error("Size mismatch in line bone weights");
        }
        auto wit = line_bone_weights.begin();
        res->lines.reserve(lines.size());
        for (const auto& li : lines) {
            res->lines.push_back({
                li(0).transformed((*wit)(0), qs),
                li(1).transformed((*wit)(1), qs)});
            // res->lines.back()(0).normalize();
            // res->lines.back()(1).normalize();
            ++wit;
        }
    }
    return res;
}

std::shared_ptr<ColoredVertexArray> ColoredVertexArray::transformed(const TransformationMatrix<float>& tm) const {
    auto res = std::make_shared<ColoredVertexArray>();
    res->material = material;
    res->triangles.reserve(triangles.size());
    for (const auto& tri : triangles) {
        res->triangles.push_back({
            tri(0).transformed(tm),
            tri(1).transformed(tm),
            tri(2).transformed(tm)});
    }
    res->lines.reserve(lines.size());
    for (const auto& li : lines) {
        res->lines.push_back({
            li(0).transformed(tm),
            li(1).transformed(tm)});
    }
    return res;
}

std::vector<CollisionTriangleSphere> ColoredVertexArray::transformed_triangles_sphere(const TransformationMatrix<float>& tm) const {
    std::vector<CollisionTriangleSphere> res;
    res.reserve(triangles.size());
    for (const auto& t : triangles) {
        FixedArray<FixedArray<float, 3>, 3> pt{
            tm * t(0).position,
            tm * t(1).position,
            tm * t(2).position};
        res.push_back(CollisionTriangleSphere{
            .bounding_sphere = BoundingSphere<float, 3>{pt},
            .plane = PlaneNd<float, 3>{pt},
            .two_sided = !material.cull_faces,
            .triangle = pt});
    }
    return res;
}

std::vector<CollisionTriangleAabb> ColoredVertexArray::transformed_triangles_bbox(const TransformationMatrix<float>& tm) const {
    std::vector<CollisionTriangleAabb> res;
    res.reserve(triangles.size());
    for (const auto& t : triangles) {
        FixedArray<FixedArray<float, 3>, 3> pt{
            tm * t(0).position,
            tm * t(1).position,
            tm * t(2).position};
        res.push_back(CollisionTriangleAabb{
            .base = CollisionTriangleSphere{
                .bounding_sphere = BoundingSphere<float, 3>{pt},
                .plane = PlaneNd<float, 3>{pt},
                .two_sided = !material.cull_faces,
                .triangle = pt
            },
            .aabb = AxisAlignedBoundingBox<float, 3>{pt}});
    }
    return res;
}

std::vector<FixedArray<FixedArray<float, 3>, 2>> ColoredVertexArray::transformed_lines(const TransformationMatrix<float>& tm) const {
    std::vector<FixedArray<FixedArray<float, 3>, 2>> res;
    res.reserve(lines.size());
    for (const auto& t : lines) {
        res.push_back(FixedArray<FixedArray<float, 3>, 2>{
            tm * t(0).position,
            tm * t(1).position});
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

void ColoredVertexArray::downsample_triangles(size_t n) {
    if (n == 0) {
        throw std::runtime_error("Cannot downsaple by a factor of 0");
    }
    if (n == 1) {
        return;
    }
    assert_true(triangle_bone_weights.empty() || (triangles.size() == triangle_bone_weights.size()));
    triangles = downsampled_array(triangles, n);
    triangle_bone_weights = downsampled_array(triangle_bone_weights, n);
}

void Mlib::sort_for_rendering(std::list<std::shared_ptr<ColoredVertexArray>>& colored_vertex_arrays) {
    colored_vertex_arrays.sort([](
        const std::shared_ptr<ColoredVertexArray>& a,
        const std::shared_ptr<ColoredVertexArray>& b)
        {
            return a->material.blend_mode < b->material.blend_mode;
        });
}

#pragma GCC pop_options
