#pragma once
#include <Mlib/Scene_Precision.hpp>
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
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const std::set<std::string>& required_prefixes,
    const std::set<VariableAndHash<std::string>>& exclude,
    std::set<VariableAndHash<std::string>>* instantiated);

}
