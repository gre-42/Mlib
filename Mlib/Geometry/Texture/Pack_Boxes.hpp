#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class FPath;

struct NameAndBoxPosition {
    FPath name;
    FixedArray<int, 2> bottom_left;
};

std::list<std::list<NameAndBoxPosition>> pack_boxes(
    const std::unordered_map<FPath, FixedArray<int, 2>>& box_sizes,
    const FixedArray<int, 2>& container_size);

}
