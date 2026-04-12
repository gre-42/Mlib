#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
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
    const Utf8Path& file_or_directory,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources,
    IDdsResources* dds_resources,
    IRaceLogic* race_logic);

}
