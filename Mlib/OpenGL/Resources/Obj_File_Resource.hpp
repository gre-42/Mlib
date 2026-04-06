#pragma once
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
class ISceneNodeResource;
class SceneNodeResources;

template <class TPos>
std::shared_ptr<ISceneNodeResource> load_renderable_obj(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources);

}
