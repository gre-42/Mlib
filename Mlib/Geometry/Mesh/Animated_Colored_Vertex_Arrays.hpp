#pragma once
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

template <class TDir, class TPos>
class OffsetAndQuaternion;
struct Bone;
template <class TPos>
struct ColoredVertexArray;
struct ColoredVertexArrayFilter;

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
        const ColoredVertexArrayFilter& filter);
    void check_consistency() const;
    void print(std::ostream& ostr) const;
};

}
