#include "Obj_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>

using namespace Mlib;

std::shared_ptr<SceneNodeResource> Mlib::load_renderable_obj(
    const std::string& filename,
    const LoadMeshConfig& cfg,
    const SceneNodeResources& scene_node_resources)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    hr->acvas->scvas = load_obj(filename, cfg);
    return hr;
}
