#include "Dff_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff_Array.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>

using namespace Mlib;

template <class TPos>
std::shared_ptr<ISceneNodeResource> Mlib::load_renderable_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    if constexpr (std::is_same_v<TPos, float>) {
        hr->acvas->scvas = load_dff(istr, name, cfg);
    } else if constexpr (std::is_same_v<TPos, double>) {
        hr->acvas->dcvas = load_dff(istr, name, cfg);
    } else {
        THROW_OR_ABORT("Unknown mesh precision");
    }
    return hr;
}

template <class TPos>
std::shared_ptr<ISceneNodeResource> Mlib::load_renderable_dff(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    if constexpr (std::is_same_v<TPos, float>) {
        hr->acvas->scvas = load_dff(filename, cfg);
    } else if constexpr (std::is_same_v<TPos, double>) {
        hr->acvas->dcvas = load_dff(filename, cfg);
    } else {
        THROW_OR_ABORT("Unknown mesh precision");
    }
    return hr;
}

namespace Mlib {

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<float>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources);

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<double>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<double>& cfg,
    const SceneNodeResources& scene_node_resources);

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<float>(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources);

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<double>(
    const std::string& filename,
    const LoadMeshConfig<double>& cfg,
    const SceneNodeResources& scene_node_resources);

}
