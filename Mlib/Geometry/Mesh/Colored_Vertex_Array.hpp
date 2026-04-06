#pragma once
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Bounding_Sphere.hpp>
#include <Mlib/Geometry/Primitives/Primitive_Dimensions.hpp>
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Misc/To_Underlying.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <cereal/access.hpp>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonAabb;
template <class TPosition>
struct CollisionLineSphere;
template <class TPosition>
struct CollisionLineAabb;
struct BoneWeight;
template <class TPos>
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos>
class OffsetAndQuaternion;
enum class RectangleTriangulationMode;
enum class DelaunayErrorBehavior;

template <class TPos>
class ColoredVertexArray {
    ColoredVertexArray() = delete;
    ColoredVertexArray(const ColoredVertexArray&) = delete;
    ColoredVertexArray& operator = (const ColoredVertexArray&) = delete;
public:
    ColoredVertexArray(
        GroupAndName name,
        const Material& material,
        const Morphology& morphology,
        ModifierBacklog modifier_backlog,
        UUVector<FixedArray<ColoredVertex<TPos>, 4>>&& quads,
        UUVector<FixedArray<ColoredVertex<TPos>, 3>>&& triangles,
        UUVector<FixedArray<ColoredVertex<TPos>, 2>>&& lines,
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights,
        UUVector<FixedArray<float, 3>>&& continous_triangle_texture_layers,
        UUVector<FixedArray<uint8_t, 3>>&& discrete_triangle_texture_layers,
        std::vector<UUVector<FixedArray<float, 3, 2>>>&& uv1,
        std::vector<UUVector<FixedArray<float, 3>>>&& cweight,
        UUVector<FixedArray<float, 3>>&& alpha,
        UUVector<FixedArray<float, 4>>&& interiormap_uvmaps,
        const ExtremalAxisAlignedBoundingBox<TPos, 3>* aabb = nullptr,
        const ExtremalBoundingSphere<TPos, 3>* bounding_sphere = nullptr);
    ~ColoredVertexArray();
    MeshMeta meta;
    UUVector<FixedArray<ColoredVertex<TPos>, to_underlying(PrimitiveDimensions::QUAD)>> quads;
    UUVector<FixedArray<ColoredVertex<TPos>, to_underlying(PrimitiveDimensions::TRIANGLE)>> triangles;
    UUVector<FixedArray<ColoredVertex<TPos>, to_underlying(PrimitiveDimensions::LINE)>> lines;
    UUVector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
    UUVector<FixedArray<float, 3>> continuous_triangle_texture_layers;
    UUVector<FixedArray<uint8_t, 3>> discrete_triangle_texture_layers;
    std::vector<UUVector<FixedArray<float, 3, 2>>> uv1;
    std::vector<UUVector<FixedArray<float, 3>>> cweight;
    UUVector<FixedArray<float, 3>> alpha;
    UUVector<FixedArray<float, 4>> interiormap_uvmaps;
    
    template <PrimitiveDimensions tdims>
    UUVector<FixedArray<ColoredVertex<TPos>, to_underlying(tdims)>>& primitives() {
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
    const UUVector<FixedArray<ColoredVertex<TPos>, to_underlying(tdims)>>& primitives() const {
        return const_cast<ColoredVertexArray*>(this)->primitives<tdims>();
    }
    size_t nelements() const;
    bool empty() const;
    UUVector<FixedArray<TPos, 3>> vertices() const;
    const ExtremalAxisAlignedBoundingBox<TPos, 3>& aabb() const;
    const ExtremalBoundingSphere<TPos, 3>& bounding_sphere() const;
    void extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const;
    void extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const;
    void extend_aabb(
        const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const;
    TPos radius(const FixedArray<TPos, 3>& center) const;
    void set_bounds(
        const AxisAlignedBoundingBox<TPos, 3>& aabb,
        const BoundingSphere<TPos, 3>& bounding_sphere);
    ScenePos max_center_distance2(BillboardId billboard_id) const;
    template <class TPosResult, class TPosTransform>
    std::shared_ptr<ColoredVertexArray<TPosResult>> transformed(
        const UUVector<OffsetAndQuaternion<float, TPosTransform>>& qs,
        const std::string& suffix) const;
    template <class TPosResult, class TPosTransform>
    std::shared_ptr<ColoredVertexArray<TPosResult>> transformed(
        const TransformationMatrix<float, TPosTransform, 3>& tm,
        const std::string& suffix) const;
    template <class TPosResult, class TPosTranslation>
    std::shared_ptr<ColoredVertexArray<TPosResult>> translated(
        const FixedArray<TPosTranslation, 3>& t,
        const std::string& suffix) const;
    template <size_t tnvertices>
    void polygon_sphere(
        std::vector<CollisionPolygonSphere<CompressedScenePos, tnvertices>>& collision_polygons) const;
    void quads_sphere(std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>& collision_polygons) const;
    void triangles_sphere(std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>& collision_polygons) const;
    template <size_t tnvertices, class TPosTransform>
    std::vector<CollisionPolygonAabb<CompressedScenePos, tnvertices>> transformed_polygon_bbox(
        const TransformationMatrix<float, TPosTransform, 3>& tm) const;
    template <class TPosTransform>
    std::vector<CollisionPolygonAabb<CompressedScenePos, 4>> transformed_quads_bbox(
        const TransformationMatrix<float, TPosTransform, 3>& tm) const;
    template <class TPosTransform>
    std::vector<CollisionPolygonAabb<CompressedScenePos, 3>> transformed_triangles_bbox(
        const TransformationMatrix<float, TPosTransform, 3>& tm) const;
    template <class TPosTransform>
    std::vector<CollisionLineAabb<CompressedScenePos>> transformed_lines_bbox(
        const TransformationMatrix<float, TPosTransform, 3>& tm) const;
    std::vector<CollisionLineSphere<CompressedScenePos>> lines_sphere() const;
    void downsample_triangles(size_t n);
    std::shared_ptr<ColoredVertexArray> generate_grind_lines(float edge_angle, float averaged_normal_angle) const;
    std::shared_ptr<ColoredVertexArray> generate_contour_edges() const;
    std::shared_ptr<ColoredVertexArray> triangulate(
        RectangleTriangulationMode mode,
        DelaunayErrorBehavior error_behavior) const;
    std::vector<std::shared_ptr<ColoredVertexArray>> split(
        float depth,
        PhysicsMaterial destination_physics_material) const;
    std::string identifier() const;
    void print_stats(std::ostream& ostr) const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(meta);
        archive(quads);
        archive(triangles);
        archive(lines);
        archive(triangle_bone_weights);
        archive(continuous_triangle_texture_layers);
        archive(discrete_triangle_texture_layers);
        archive(uv1);
        archive(cweight);
        archive(alpha);
        archive(interiormap_uvmaps);
    }
    // From: https://github.com/USCiLab/cereal/issues/102
    template<typename Archive>
    static void load_and_construct(
        Archive& archive,
        cereal::construct<ColoredVertexArray>& construct)
    {
        MeshMeta meta;
        UUVector<FixedArray<ColoredVertex<TPos>, 4>> quads;
        UUVector<FixedArray<ColoredVertex<TPos>, 3>> triangles;
        UUVector<FixedArray<ColoredVertex<TPos>, 2>> lines;
        UUVector<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
        UUVector<FixedArray<float, 3>> continuous_triangle_texture_layers;
        UUVector<FixedArray<uint8_t, 3>> discrete_triangle_texture_layers;
        std::vector<UUVector<FixedArray<float, 3, 2>>> uv1;
        std::vector<UUVector<FixedArray<float, 3>>> cweight;
        UUVector<FixedArray<float, 3>> alpha;
        UUVector<FixedArray<float, 4>> interiormap_uvmaps;

        archive(meta);
        archive(quads);
        archive(triangles);
        archive(lines);
        archive(triangle_bone_weights);
        archive(continuous_triangle_texture_layers);
        archive(discrete_triangle_texture_layers);
        archive(uv1);
        archive(cweight);
        archive(alpha);
        archive(interiormap_uvmaps);

        construct(
            std::move(meta.name),
            std::move(meta.material),
            std::move(meta.morphology),
            std::move(meta.modifier_backlog),
            std::move(quads),
            std::move(triangles),
            std::move(lines),
            std::move(triangle_bone_weights),
            std::move(continuous_triangle_texture_layers),
            std::move(discrete_triangle_texture_layers),
            std::move(uv1),
            std::move(cweight),
            std::move(alpha),
            std::move(interiormap_uvmaps));
    }
private:
    template <PrimitiveDimensions tfirst_dim>
    size_t nelements_from() const;
    template <PrimitiveDimensions tfirst_dim>
    bool empty_from() const;
    mutable std::optional<ExtremalAxisAlignedBoundingBox<TPos, 3>> aabb_;
    mutable std::optional<ExtremalBoundingSphere<TPos, 3>> bounding_sphere_;
    mutable SafeAtomicRecursiveSharedMutex aabb_mutex_;
    mutable SafeAtomicRecursiveSharedMutex bounding_sphere_mutex_;
    mutable std::atomic_bool aabb_has_value_;
    mutable std::atomic_bool bounding_sphere_has_value_;
};

}
