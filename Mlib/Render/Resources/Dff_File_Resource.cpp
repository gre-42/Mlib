#include "Dff_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff_Array.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>

using namespace Mlib;

std::shared_ptr<ISceneNodeResource> Mlib::load_renderable_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    hr->acvas->scvas = load_dff(istr, name, cfg);
    return hr;
}

std::shared_ptr<ISceneNodeResource> Mlib::load_renderable_dff(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    hr->acvas->scvas = load_dff(filename, cfg);
    return hr;
}
