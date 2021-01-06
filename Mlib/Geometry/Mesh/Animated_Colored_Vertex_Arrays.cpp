#include "Animated_Colored_Vertex_Arrays.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Quaternion.hpp>

using namespace Mlib;

std::vector<OffsetAndQuaternion<float>> AnimatedColoredVertexArrays::vectorize_joint_poses(
    const std::map<std::string, OffsetAndQuaternion<float>>& poses) const
{
    std::vector<OffsetAndQuaternion<float>> ms(bone_indices.size());
    for (auto& m : ms) {
        m.offset() = fixed_nans<float, 3>();
    }
    for (const auto& p : poses) {
        auto it = bone_indices.find(p.first);
        if (it == bone_indices.end()) {
            throw std::runtime_error("vectorize_joint_poses: Could not find bone with name " + p.first);
        }
        ms.at(it->second) = p.second;
    }
    for (const auto& m : ms) {
        if (any(isnan(m.offset()))) {
            throw std::runtime_error("Pose contains NAN values or was not set");
        }
    }
    return ms;
}

void AnimatedColoredVertexArrays::check_consistency() const {
    assert_true(bone_indices.empty() == !skeleton);
    for (const auto& cva : cvas) {
        assert_true(cva->triangle_bone_weights.empty() == !skeleton);
    }
}
