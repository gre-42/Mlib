#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <map>
#include <memory>
#include <vector>

namespace Mlib {

struct Bone {
    size_t index;
    FixedArray<float, 4, 4> transformation;
    std::vector<std::unique_ptr<Bone>> children;
    std::vector<FixedArray<float, 4, 4>> absolutify(
        const std::vector<FixedArray<float, 4, 4>>& transformations);
private:
    void absolutify(
        const std::vector<FixedArray<float, 4, 4>>& transformations,
        const FixedArray<float, 4, 4>& parent_transformation,
        std::vector<FixedArray<float, 4, 4>>& result);
};

struct AnimatedColoredVertexArrays {
    Bone skeleton;
    std::map<std::string, size_t> bone_indices;
    std::list<std::shared_ptr<ColoredVertexArray>> cvas;
};

}
