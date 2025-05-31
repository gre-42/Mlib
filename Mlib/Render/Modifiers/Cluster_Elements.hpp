#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <string>
#include <vector>

namespace Mlib {

template <class T>
class VariableAndHash;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class SceneNodeResources;
class GroupAndName;
enum class RenderingDynamics;
class SquaredStepDistances;

void cluster_elements(
    const std::vector<VariableAndHash<std::string>>& resource_names,
    SceneNodeResources& scene_node_resources,
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
    const FixedArray<float, 3>& width,
    const SquaredStepDistances& center_distances2,
    RenderingDynamics rendering_dynamics,
    std::list<VariableAndHash<std::string>>& added_scene_node_resources,
    std::list<VariableAndHash<std::string>>& added_instantiables);

}
