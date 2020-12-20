#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/String.hpp>
#include <memory>
#include <regex>
#include <vector>

namespace Mlib {

std::list<std::shared_ptr<ColoredVertexArray>> load_obj(
    const std::string& filename,
    bool is_small,
    BlendMode blend_mode,
    bool blend_cull_faces,
    OccludedType occluded_type,
    OccluderType occluder_type,
    bool occluded_by_black,
    AggregateMode aggregate_mode,
    TransformationMode transformation_mode,
    bool apply_static_lighting,
    bool werror);

}
