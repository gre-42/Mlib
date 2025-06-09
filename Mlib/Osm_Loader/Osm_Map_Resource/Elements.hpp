#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>

namespace Mlib {

struct Node {
    FixedArray<CompressedScenePos, 2> position;
    Map<std::string, std::string> tags;
};

struct Way {
    std::list<std::string> nd;
    Map<std::string, std::string> tags;
};

}
