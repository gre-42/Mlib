#pragma once
#include <memory>
#include <string>

namespace Mlib {

struct LoadMeshConfig;
class SceneNodeResource;
class SceneNodeResources;

std::shared_ptr<SceneNodeResource> load_renderable_obj(
    const std::string& filename,
    const LoadMeshConfig& cfg,
    const SceneNodeResources& scene_node_resources);

}
