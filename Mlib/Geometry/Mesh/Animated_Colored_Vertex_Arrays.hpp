#pragma once
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <map>
#include <memory>

namespace Mlib {

struct AnimatedColoredVertexArrays {
    std::unique_ptr<Bone> skeleton;
    std::map<std::string, size_t> bone_indices;
    std::list<std::shared_ptr<ColoredVertexArray>> cvas;
    std::vector<FixedArray<float, 4, 4>> vectorize_joint_poses(const std::map<std::string, FixedArray<float, 4, 4>>& poses) const;
};

}
