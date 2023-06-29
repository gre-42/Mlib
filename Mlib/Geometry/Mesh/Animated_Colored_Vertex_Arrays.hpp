#pragma once
#include <cstddef>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos>
class OffsetAndQuaternion;
struct Bone;
template <class TPos>
class ColoredVertexArray;
struct ColoredVertexArrayFilter;
struct Material;
enum class PhysicsMaterial;
struct UvTile;

struct AnimatedColoredVertexArrays {
    AnimatedColoredVertexArrays();
    ~AnimatedColoredVertexArrays();
    std::shared_ptr<Bone> skeleton;
    std::map<std::string, size_t> bone_indices;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> scvas;
    std::list<std::shared_ptr<ColoredVertexArray<double>>> dcvas;
    std::vector<OffsetAndQuaternion<float, float>> vectorize_joint_poses(
        const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) const;
    std::shared_ptr<AnimatedColoredVertexArrays> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const;
    void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter);
    void merge_materials(
        const std::string& merged_array_name,
        const Material& merged_material,
        PhysicsMaterial physics_material,
        const std::map<std::string, UvTile>& uv_tiles);
    void check_consistency() const;
    void print(std::ostream& ostr) const;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(skeleton);
        archive(bone_indices);
        archive(scvas);
        archive(dcvas);
    }
};

}
