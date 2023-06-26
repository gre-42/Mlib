#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> load_obj(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg);

}
