#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <string>

namespace Mlib {

template <class TResourcePos, class TInstancePos>
struct PssgArrays;
class ColoredVertexArrayFilters;
class SceneNodeResources;

template <class TResourcePos>
void load_renderable_pssg(
    const PssgArrays<TResourcePos, ScenePos>& arrays,
    const ColoredVertexArrayFilters& filters,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

}
