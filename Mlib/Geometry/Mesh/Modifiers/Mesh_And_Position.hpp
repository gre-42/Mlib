#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <memory>

namespace Mlib {

template <class TPos>
struct MeshAndPosition {
    std::shared_ptr<ColoredVertexArray<TPos>> cva;
    FixedArray<TPos, 3> position;
};

}
