#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos>
class OffsetAndQuaternion;
struct Bone;
template <class TPos>
class ColoredVertexArray;
struct ColoredVertexArrayFilter;
enum class PhysicsMaterial: uint32_t;
enum class SmoothnessTarget;
enum class RectangleTriangulationMode;
enum class DelaunayErrorBehavior;

struct AnimatedColoredVertexArrays {
    AnimatedColoredVertexArrays();
    AnimatedColoredVertexArrays(
        const AnimatedColoredVertexArrays& other,
        const ColoredVertexArrayFilter& filter);
    ~AnimatedColoredVertexArrays();
    std::shared_ptr<Bone> skeleton;
    StringWithHashUnorderedMap<size_t> bone_indices;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> scvas;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dcvas;
    void insert(const AnimatedColoredVertexArrays& other);
    template <class TPos>
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas();
    UUVector<OffsetAndQuaternion<float, float>> vectorize_joint_poses(
        const StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>>& poses) const;
    std::shared_ptr<AnimatedColoredVertexArrays> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const;
    void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter);
    void smoothen_edges(
        SmoothnessTarget target,
        float smoothness,
        size_t niterations,
        float decay = 0.97f);
    std::shared_ptr<AnimatedColoredVertexArrays> triangulate(
        RectangleTriangulationMode mode,
        DelaunayErrorBehavior error_behavior) const;
    void check_consistency() const;
    void print(std::ostream& ostr) const;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(skeleton);
        archive(bone_indices.elements());
        archive(scvas);
        archive(dcvas);
    }
};

}
