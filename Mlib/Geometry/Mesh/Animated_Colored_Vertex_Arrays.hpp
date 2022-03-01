#pragma once
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <vector>

namespace Mlib {

template <class TData>
class OffsetAndQuaternion;
struct Bone;
struct ColoredVertexArray;
struct ColoredVertexArrayFilter;

struct AnimatedColoredVertexArrays {
    AnimatedColoredVertexArrays();
    ~AnimatedColoredVertexArrays();
    std::shared_ptr<Bone> skeleton;
    std::map<std::string, size_t> bone_indices;
    std::list<std::shared_ptr<ColoredVertexArray>> cvas;
    std::vector<OffsetAndQuaternion<float>> vectorize_joint_poses(
        const std::map<std::string, OffsetAndQuaternion<float>>& poses) const;
    std::shared_ptr<AnimatedColoredVertexArrays> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter);
    void check_consistency() const;
    void print(std::ostream& ostr) const;
};

}
