#pragma once
#include <memory>
#include <string>

namespace Mlib {

struct LoadMeshConfig;
class ISceneNodeResource;
class SceneNodeResources;

std::shared_ptr<ISceneNodeResource> load_renderable_obj(
    const std::string& filename,
    const LoadMeshConfig& cfg,
    const SceneNodeResources& scene_node_resources);

}
