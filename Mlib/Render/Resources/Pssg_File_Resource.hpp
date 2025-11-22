#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
template <class TResourcePos, class TInstancePos>
struct PssgArrays;
class ColoredVertexArrayFilters;
class SceneNodeResources;

template <class TResourcePos>
void load_renderable_pssg(
    const PssgArrays<TResourcePos, ScenePos>& arrays,
    const ColoredVertexArrayFilters& filters,
    SceneNodeResources& scene_node_resources,
    std::list<VariableAndHash<std::string>>& added_scene_node_resources,
    std::list<VariableAndHash<std::string>>& added_instantiables);

}
