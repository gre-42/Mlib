#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
class ISceneNodeResource;
class SceneNodeResources;

std::shared_ptr<ISceneNodeResource> load_renderable_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources);

std::shared_ptr<ISceneNodeResource> load_renderable_dff(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources);

}
