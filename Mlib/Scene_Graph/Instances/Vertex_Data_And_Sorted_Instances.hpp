#pragma once
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <memory>

namespace Mlib {

class IGpuVertexData;

struct VertexDataAndSortedInstances {
    std::shared_ptr<IGpuVertexData> vertex_data;
    SortedVertexArrayInstances instances;
};

}
