#pragma once
#include <iosfwd>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
class ISceneNodeResource;
class SceneNodeResources;
class DrawDistanceDb;
class IDdsResources;

template <class TPos>
void load_renderable_pssg(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPos>& cfg,
    IDdsResources* dds_resources,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

template <class TPos>
void load_renderable_pssg(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    IDdsResources* dds_resources,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

}
