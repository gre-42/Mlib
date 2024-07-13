#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
class ISceneNodeResource;
class SceneNodeResources;

template <class TPos>
std::shared_ptr<ISceneNodeResource> load_renderable_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources);

template <class TPos>
std::shared_ptr<ISceneNodeResource> load_renderable_dff(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources);

}
