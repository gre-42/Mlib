#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
struct AnimatedColoredVertexArrays;

std::shared_ptr<AnimatedColoredVertexArrays> load_mhx2(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg);

}
