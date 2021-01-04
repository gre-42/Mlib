#include "Animated_Colored_Vertex_Arrays.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

std::vector<FixedArray<float, 4, 4>> AnimatedColoredVertexArrays::vectorize_joint_poses(
    const std::map<std::string, FixedArray<float, 4, 4>>& poses) const
{
    std::vector<FixedArray<float, 4, 4>> ms(bone_indices.size());
    for (auto& m : ms) {
        m = fixed_nans<float, 4, 4>();
    }
    for (const auto& p : poses) {
        auto it = bone_indices.find(p.first);
        if (it == bone_indices.end()) {
            throw std::runtime_error("vectorize_joint_poses: Could not find bone with name " + p.first);
        }
        ms.at(it->second) = p.second;
    }
    for (const auto& m : ms) {
        if (any(isnan(m))) {
            throw std::runtime_error("Pose contains NAN values or was not set");
        }
    }
    return ms;
}
