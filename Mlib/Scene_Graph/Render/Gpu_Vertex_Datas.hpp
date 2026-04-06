#pragma once
#include <list>
#include <memory>

namespace Mlib {

class IGpuVertexData;
enum class TransformationMode;

struct GpuVertexDatas {
    std::list<std::shared_ptr<IGpuVertexData>> vertex_data;
    TransformationMode transformation_mode;
};

}
