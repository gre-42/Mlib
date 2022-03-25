#pragma once
#include <Mlib/Geometry/Material.hpp>
#include <cereal/access.hpp>
#include <iosfwd>
#include <memory>
#include <vector>

namespace Mlib {

template <class TData, size_t tsize>
class TransformationMatrix;
struct CollisionTriangleSphere;
struct CollisionLineAabb;
struct CollisionTriangleAabb;
struct BoneWeight;
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TData>
class OffsetAndQuaternion;
enum class PhysicsMaterial;

struct ColoredVertexArray {
    ColoredVertexArray() = delete;
    ColoredVertexArray(const ColoredVertexArray&) = delete;
    ColoredVertexArray& operator = (const ColoredVertexArray&) = delete;
    ColoredVertexArray(ColoredVertexArray&&) = default;
    ColoredVertexArray& operator = (ColoredVertexArray&&) = default;
    ColoredVertexArray(
        const std::string& name,
        const Material& material,
        PhysicsMaterial physics_material,
        std::vector<FixedArray<ColoredVertex, 3>>&& triangles,
        std::vector<FixedArray<ColoredVertex, 2>>&& lines,
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
        std::vector<FixedArray<std::vector<BoneWeight>, 2>>&& line_bone_weights);
    ~ColoredVertexArray();
    std::string name;
    Material material;
    PhysicsMaterial physics_material;
    std::vector<FixedArray<ColoredVertex, 3>> triangles;
    std::vector<FixedArray<ColoredVertex, 2>> lines;
    std::vector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
    std::vector<FixedArray<std::vector<BoneWeight>, 2>> line_bone_weights;
    
    std::vector<FixedArray<float, 3>> vertices() const;
    std::shared_ptr<ColoredVertexArray> transformed(
        const std::vector<OffsetAndQuaternion<float>>& qs,
        const std::string& suffix) const;
    std::shared_ptr<ColoredVertexArray> transformed(
        const TransformationMatrix<float, 3>& tm,
        const std::string& suffix) const;
    std::vector<CollisionTriangleSphere> transformed_triangles_sphere(
        const TransformationMatrix<float, 3>& tm) const;
    std::vector<CollisionTriangleAabb> transformed_triangles_bbox(
        const TransformationMatrix<float, 3>& tm) const;
    std::vector<CollisionLineAabb> transformed_lines_bbox(const TransformationMatrix<float, 3>& tm) const;
    std::vector<FixedArray<FixedArray<float, 3>, 2>> transformed_lines(const TransformationMatrix<float, 3>& tm) const;
    void downsample_triangles(size_t n);
    ColoredVertexArray generate_grind_lines(float edge_angle, float averaged_normal_angle) const;
    ColoredVertexArray generate_contour_edges() const;
    void print(std::ostream& ostr) const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(name);
        archive(material);
        archive(physics_material);
        archive(triangles);
        archive(lines);
        archive(triangle_bone_weights);
        archive(line_bone_weights);
    }
    // From: https://github.com/USCiLab/cereal/issues/102
    template<typename Archive>
    static void load_and_construct(
        Archive& archive,
        cereal::construct<ColoredVertexArray>& construct)
    {
        std::string name;
        Material material;
        PhysicsMaterial physics_material;
        std::vector<FixedArray<ColoredVertex, 3>> triangles;
        std::vector<FixedArray<ColoredVertex, 2>> lines;
        std::vector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
        std::vector<FixedArray<std::vector<BoneWeight>, 2>> line_bone_weights;

        archive(name);
        archive(material);
        archive(physics_material);
        archive(triangles);
        archive(lines);
        archive(triangle_bone_weights);
        archive(line_bone_weights);

        construct(
            name,
            material,
            physics_material,
            std::move(triangles),
            std::move(lines),
            std::move(triangle_bone_weights),
            std::move(line_bone_weights));
    }
};

void sort_for_rendering(std::list<std::shared_ptr<ColoredVertexArray>>& colored_vertex_arrays);

}
