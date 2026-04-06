#pragma once
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
class ISceneNodeResource;
class SceneNodeResources;
class IDdsResources;
class IRaceLogic;

template <class TPos>
std::shared_ptr<ISceneNodeResource> load_renderable_kn5(
    const std::string& file_or_directory,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources,
    IDdsResources* dds_resources,
    IRaceLogic* race_logic);

}
