#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
struct PositionAndMeshes {
    FixedArray<TPos, 3> position;
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> cvas;
};

}
