#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Geometry/Modifier_Backlog.hpp>
#include <Mlib/Geometry/Primitive_Dimensions.hpp>
#include <Mlib/Ignore_Copy.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <Mlib/To_Underlying.hpp>
#include <cereal/access.hpp>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TData, size_t tnvertices>
struct CollisionPolygonAabb;
template <class TData>
struct CollisionLineSphere;
template <class TData>
struct CollisionLineAabb;
struct BoneWeight;
template <class TPos>
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos>
class OffsetAndQuaternion;
enum class PhysicsMaterial: uint32_t;

template <class TPos>
class ColoredVertexArray {
    ColoredVertexArray() = delete;
    ColoredVertexArray(const ColoredVertexArray&) = delete;
    ColoredVertexArray& operator = (const ColoredVertexArray&) = delete;
public:
    ColoredVertexArray(ColoredVertexArray&&) = default;
    ColoredVertexArray& operator = (ColoredVertexArray&&) = default;
    ColoredVertexArray(
        std::string name,
        const Material& material,
        PhysicsMaterial physics_material,
        ModifierBacklog modifier_backlog,
        std::vector<FixedArray<ColoredVertex<TPos>, 4>>&& quads,
        std::vector<FixedArray<ColoredVertex<TPos>, 3>>&& triangles,
        std::vector<FixedArray<ColoredVertex<TPos>, 2>>&& lines,
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
        std::vector<FixedArray<float, 3>>&& continous_triangle_texture_layers,
        std::vector<FixedArray<uint8_t, 3>>&& discrete_triangle_texture_layers,
        const AxisAlignedBoundingBox<TPos, 3>* aabb = nullptr,
        const BoundingSphere<TPos, 3>* bounding_sphere = nullptr);
    ~ColoredVertexArray();
    std::string name;
    Material material;
    PhysicsMaterial physics_material;
    ModifierBacklog modifier_backlog;
    std::vector<FixedArray<ColoredVertex<TPos>, to_underlying(PrimitiveDimensions::QUAD)>> quads;
    std::vector<FixedArray<ColoredVertex<TPos>, to_underlying(PrimitiveDimensions::TRIANGLE)>> triangles;
    std::vector<FixedArray<ColoredVertex<TPos>, to_underlying(PrimitiveDimensions::LINE)>> lines;
    std::vector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
    std::vector<FixedArray<float, 3>> continuous_triangle_texture_layers;
    std::vector<FixedArray<uint8_t, 3>> discrete_triangle_texture_layers;
    
    template <PrimitiveDimensions tdims>
    std::vector<FixedArray<ColoredVertex<TPos>, to_underlying(tdims)>>& primitives() {
        if constexpr (tdims == PrimitiveDimensions::QUAD) {
            return quads;
        } else if constexpr (tdims == PrimitiveDimensions::TRIANGLE) {
            return triangles;
        } else if constexpr (tdims == PrimitiveDimensions::LINE) {
            return lines;
        } else {
            static_assert(tdims == PrimitiveDimensions::LINE, "Unknown primitive dimension");
        }
    }
    template <PrimitiveDimensions tdims>
    const std::vector<FixedArray<ColoredVertex<TPos>, to_underlying(tdims)>>& primitives() const {
        return const_cast<ColoredVertexArray*>(this)->primitives<tdims>();
    }
    bool empty() const;
    std::vector<FixedArray<TPos, 3>> vertices() const;
    AxisAlignedBoundingBox<TPos, 3> aabb() const;
    BoundingSphere<TPos, 3> bounding_sphere() const;
    void set_bounds(
        const AxisAlignedBoundingBox<TPos, 3>& aabb,
        const BoundingSphere<TPos, 3>& bounding_sphere);
    double max_center_distance(uint32_t billboard_id) const;
    template <class TPosResult, class TPosTransform>
    std::shared_ptr<ColoredVertexArray<TPosResult>> transformed(
        const std::vector<OffsetAndQuaternion<float, TPosTransform>>& qs,
        const std::string& suffix) const;
    template <class TPosResult, class TPosTransform>
    std::shared_ptr<ColoredVertexArray<TPosResult>> transformed(
        const TransformationMatrix<float, TPosTransform, 3>& tm,
        const std::string& suffix) const;
    void quads_sphere(std::vector<CollisionPolygonSphere<TPos, 4>>& collision_polygons) const;
    void triangles_sphere(std::vector<CollisionPolygonSphere<TPos, 3>>& collision_polygons) const;
    std::vector<CollisionPolygonAabb<double, 3>> transformed_triangles_bbox(
        const TransformationMatrix<float, double, 3>& tm) const;
    std::vector<CollisionLineAabb<double>> transformed_lines_bbox(
        const TransformationMatrix<float, double, 3>& tm) const;
    std::vector<CollisionLineSphere<TPos>> lines_sphere() const;
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
        archive(modifier_backlog);
        archive(quads);
        archive(triangles);
        archive(lines);
        archive(triangle_bone_weights);
        archive(continuous_triangle_texture_layers);
        archive(discrete_triangle_texture_layers);
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
        ModifierBacklog modifier_backlog;
        std::vector<FixedArray<ColoredVertex<TPos>, 4>> quads;
        std::vector<FixedArray<ColoredVertex<TPos>, 3>> triangles;
        std::vector<FixedArray<ColoredVertex<TPos>, 2>> lines;
        std::vector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
        std::vector<FixedArray<float, 3>> continuous_triangle_texture_layers;
        std::vector<FixedArray<uint8_t, 3>> discrete_triangle_texture_layers;

        archive(name);
        archive(material);
        archive(physics_material);
        archive(modifier_backlog);
        archive(quads);
        archive(triangles);
        archive(lines);
        archive(triangle_bone_weights);
        archive(continuous_triangle_texture_layers);
        archive(discrete_triangle_texture_layers);

        construct(
            name,
            material,
            physics_material,
            modifier_backlog,
            std::move(quads),
            std::move(triangles),
            std::move(lines),
            std::move(triangle_bone_weights),
            std::move(continuous_triangle_texture_layers),
            std::move(discrete_triangle_texture_layers));
    }
private:
    template <PrimitiveDimensions tfirst_dim>
    bool empty_from() const;
    mutable std::optional<AxisAlignedBoundingBox<TPos, 3>> aabb_;
    mutable std::optional<BoundingSphere<TPos, 3>> bounding_sphere_;
    mutable IgnoreCopy<SafeSharedMutex> aabb_mutex_;
    mutable IgnoreCopy<SafeSharedMutex> bounding_sphere_mutex_;
};

}
