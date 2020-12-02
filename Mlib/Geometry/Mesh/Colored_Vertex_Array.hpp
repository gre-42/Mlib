#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <memory>
#include <vector>

namespace Mlib {

struct ColoredVertexArray {
    ColoredVertexArray() = default;
    ColoredVertexArray(const ColoredVertexArray&) = delete;
    ColoredVertexArray& operator = (const ColoredVertexArray&) = delete;
    ColoredVertexArray(
        const std::string& name,
        const Material& material,
        std::vector<FixedArray<ColoredVertex, 3>>&& triangles,
        std::vector<FixedArray<ColoredVertex, 2>>&& lines);
    std::string name;
    Material material;
    std::vector<FixedArray<ColoredVertex, 3>> triangles;
    std::vector<FixedArray<ColoredVertex, 2>> lines;
    std::vector<FixedArray<float, 3>> vertices() const;
    std::shared_ptr<ColoredVertexArray> transformed(const FixedArray<float, 4, 4>& m) const;
    std::vector<CollisionTriangleSphere> transformed_triangles_sphere(const FixedArray<float, 4, 4>& m) const;
    std::vector<CollisionTriangleBbox> transformed_triangles_bbox(const FixedArray<float, 4, 4>& m) const;
    std::vector<FixedArray<FixedArray<float, 3>, 2>> transformed_lines(const FixedArray<float, 4, 4>& m) const;
};

void sort_for_rendering(std::list<std::shared_ptr<ColoredVertexArray>>& colored_vertex_arrays);

}
