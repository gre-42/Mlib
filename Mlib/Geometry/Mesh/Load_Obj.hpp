#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <memory>

namespace Mlib {

struct LoadMeshConfig;

std::list<std::shared_ptr<ColoredVertexArray>> load_obj(
    const std::string& filename,
    const LoadMeshConfig& cfg);

}
