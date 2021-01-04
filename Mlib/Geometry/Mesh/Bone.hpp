#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>
#include <vector>

namespace Mlib {

struct Bone {
    size_t index;
    FixedArray<float, 4, 4> initial_absolute_transformation;
    std::vector<std::unique_ptr<Bone>> children;
    std::vector<FixedArray<float, 4, 4>> absolutify(
        const std::vector<FixedArray<float, 4, 4>>& transformations);
private:
    void absolutify(
        const std::vector<FixedArray<float, 4, 4>>& transformations,
        const FixedArray<float, 4, 4>& parent_transformation,
        std::vector<FixedArray<float, 4, 4>>& result);
};

}
