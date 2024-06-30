#pragma once
#include <string>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
class ISceneNodeResource;
class SceneNodeResources;

std::shared_ptr<ISceneNodeResource> load_renderable_dff(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources);

}
