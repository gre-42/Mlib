#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct NameAndBoxPosition {
    VariableAndHash<std::string> name;
    FixedArray<int, 2> bottom_left;
};

std::list<std::list<NameAndBoxPosition>> pack_boxes(
    const std::unordered_map<VariableAndHash<std::string>, FixedArray<int, 2>>& box_sizes,
    const FixedArray<int, 2>& container_size);

}
