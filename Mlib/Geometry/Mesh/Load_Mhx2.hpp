#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <memory>

namespace Mlib {

std::list<std::shared_ptr<ColoredVertexArray>> load_mhx2(
    const std::string& filename,
    bool is_small,
    BlendMode blend_mode,
    bool cull_faces,
    OccludedType occluded_type,
    OccluderType occluder_type,
    bool occluded_by_black,
    AggregateMode aggregate_mode,
    TransformationMode transformation_mode,
    bool werror);

}
