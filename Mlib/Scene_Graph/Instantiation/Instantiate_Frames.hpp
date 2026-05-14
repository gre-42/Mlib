#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <chrono>
#include <list>
#include <optional>
#include <set>
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
class Scene;
template <class TPosition>
struct InstanceInformation;
class SceneNodeResources;
class RenderingResources;

void instantiate(
    Scene& scene,
    const InstanceInformation<ScenePos>& info,
    std::optional<std::chrono::steady_clock::time_point> time,
    SceneNodeResources& scene_node_resources,
    #ifndef WITHOUT_GRAPHICS
    RenderingResources& rendering_resources,
    #endif
    const std::set<std::string>& required_prefixes,
    const std::set<VariableAndHash<std::string>>& exclude,
    std::set<VariableAndHash<std::string>>* instantiated_resources,
    std::list<VariableAndHash<std::string>>* instantiated_root_nodes);

}
