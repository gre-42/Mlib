#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Ignore_Copy.hpp>
#include <cereal/access.hpp>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <shared_mutex>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct CollisionTriangleSphere;
struct CollisionTriangleAabb;
struct CollisionLineSphere;
struct CollisionLineAabb;
struct BoneWeight;
template <class TPos>
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos>
class OffsetAndQuaternion;
enum class PhysicsMaterial;

template <class TPos>
class ColoredVertexArray {
    ColoredVertexArray() = delete;
    ColoredVertexArray(const ColoredVertexArray&) = delete;
    ColoredVertexArray& operator = (const ColoredVertexArray&) = delete;
public:
    ColoredVertexArray(ColoredVertexArray&&) = default;
    ColoredVertexArray& operator = (ColoredVertexArray&&) = default;
    ColoredVertexArray(
        const std::string& name,
        Material material,
        PhysicsMaterial physics_material,
        std::vector<FixedArray<ColoredVertex<TPos>, 3>>&& triangles,
        std::vector<FixedArray<ColoredVertex<TPos>, 2>>&& lines,
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
        std::vector<FixedArray<std::vector<BoneWeight>, 2>>&& line_bone_weights);
    ~ColoredVertexArray();
    std::string name;
    Material material;
    PhysicsMaterial physics_material;
    std::vector<FixedArray<ColoredVertex<TPos>, 3>> triangles;
    std::vector<FixedArray<ColoredVertex<TPos>, 2>> lines;
    std::vector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
    std::vector<FixedArray<std::vector<BoneWeight>, 2>> line_bone_weights;
    
    std::vector<FixedArray<TPos, 3>> vertices() const;
    AxisAlignedBoundingBox<TPos, 3> aabb() const;
    double max_center_distance(uint32_t billboard_id) const;
    template <class TPosResult, class TPosTransform>
    std::shared_ptr<ColoredVertexArray<TPosResult>> transformed(
        const std::vector<OffsetAndQuaternion<float, TPosTransform>>& qs,
        const std::string& suffix) const;
    template <class TPosResult, class TPosTransform>
    std::shared_ptr<ColoredVertexArray<TPosResult>> transformed(
        const TransformationMatrix<float, TPosTransform, 3>& tm,
        const std::string& suffix) const;
    void transformed_triangles_sphere(
        std::vector<CollisionTriangleSphere>& transformed,
        const TransformationMatrix<float, double, 3>& tm) const;
    std::vector<CollisionTriangleAabb> transformed_triangles_bbox(
        const TransformationMatrix<float, double, 3>& tm) const;
    std::vector<CollisionLineAabb> transformed_lines_bbox(
        const TransformationMatrix<float, double, 3>& tm) const;
    std::vector<CollisionLineSphere> transformed_lines_sphere(
        const TransformationMatrix<float, double, 3>& tm) const;
    void downsample_triangles(size_t n);
    ColoredVertexArray generate_grind_lines(TPos edge_angle, TPos averaged_normal_angle) const;
    ColoredVertexArray generate_contour_edges() const;
    std::vector<std::shared_ptr<ColoredVertexArray>> split(
        float depth,
        PhysicsMaterial destination_physics_material) const;
    std::string identifier() const;
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
        std::vector<FixedArray<ColoredVertex<TPos>, 3>> triangles;
        std::vector<FixedArray<ColoredVertex<TPos>, 2>> lines;
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
private:
    mutable std::optional<AxisAlignedBoundingBox<TPos, 3>> aabb_;
    mutable IgnoreCopy<std::shared_mutex> aabb_mutex_;
};

}
