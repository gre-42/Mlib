#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SceneNodeResources;
class GroupAndName;
enum class RenderingDynamics;
class SquaredStepDistances;

void cluster_elements(
    const std::vector<std::string>& resource_names,
    SceneNodeResources& scene_node_resources,
    const FixedArray<float, 3>& width,
    const SquaredStepDistances& center_distances2,
    RenderingDynamics rendering_dynamics,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

}
