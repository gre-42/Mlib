#pragma once
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <map>
#include <memory>

namespace Mlib {

template <class TData>
class OffsetAndQuaternion;

struct AnimatedColoredVertexArrays {
    std::shared_ptr<Bone> skeleton;
    std::map<std::string, size_t> bone_indices;
    std::list<std::shared_ptr<ColoredVertexArray>> cvas;
    std::vector<OffsetAndQuaternion<float>> vectorize_joint_poses(
        const std::map<std::string, OffsetAndQuaternion<float>>& poses) const;
    void check_consistency() const;
};

}
