#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/BoneWeight.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <class TData>
class TransformationMatrix;

struct ColoredVertexArray {
    ColoredVertexArray() = default;
    ColoredVertexArray(const ColoredVertexArray&) = delete;
    ColoredVertexArray& operator = (const ColoredVertexArray&) = delete;
    ColoredVertexArray(
        const std::string& name,
        const Material& material,
        std::vector<FixedArray<ColoredVertex, 3>>&& triangles,
        std::vector<FixedArray<ColoredVertex, 2>>&& lines,
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
        std::vector<FixedArray<std::vector<BoneWeight>, 2>>&& line_bone_weights);
    std::string name;
    Material material;
    std::vector<FixedArray<ColoredVertex, 3>> triangles;
    std::vector<FixedArray<ColoredVertex, 2>> lines;
    std::vector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
    std::vector<FixedArray<std::vector<BoneWeight>, 2>> line_bone_weights;
    std::vector<FixedArray<float, 3>> vertices() const;
    std::shared_ptr<ColoredVertexArray> transformed(const std::vector<OffsetAndQuaternion<float>>& qs) const;
    std::shared_ptr<ColoredVertexArray> transformed(const TransformationMatrix<float>& tm) const;
    std::vector<CollisionTriangleSphere> transformed_triangles_sphere(const TransformationMatrix<float>& tm) const;
    std::vector<CollisionTriangleAabb> transformed_triangles_bbox(const TransformationMatrix<float>& tm) const;
    std::vector<FixedArray<FixedArray<float, 3>, 2>> transformed_lines(const TransformationMatrix<float>& tm) const;
    void downsample_triangles(size_t n);
};

void sort_for_rendering(std::list<std::shared_ptr<ColoredVertexArray>>& colored_vertex_arrays);

}
