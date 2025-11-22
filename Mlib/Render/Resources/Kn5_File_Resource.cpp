#include "Kn5_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Kn5_Array.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

template <class TPos>
std::shared_ptr<ISceneNodeResource> Mlib::load_renderable_kn5(
    const std::string& file_or_directory,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources,
    IDdsResources* dds_resources,
    IRaceLogic* race_logic)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    hr->acvas->template cvas<TPos>() = load_kn5_array<TPos>(
        file_or_directory,
        cfg,
        dds_resources,
        race_logic);
    return hr;
}

namespace Mlib {
template std::shared_ptr<ISceneNodeResource> load_renderable_kn5(
    const std::string& file_or_directory,
    const LoadMeshConfig<float>&,
    const SceneNodeResources&,
    IDdsResources*,
    IRaceLogic*);
template std::shared_ptr<ISceneNodeResource> load_renderable_kn5(
    const std::string& file_or_directory,
    const LoadMeshConfig<CompressedScenePos>&,
    const SceneNodeResources&,
    IDdsResources*,
    IRaceLogic*);
}
