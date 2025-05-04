#pragma once
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
class SceneNodeResources;
struct ColoredVertexArrayFilter;

void add_shade_auto_modifier(
    const VariableAndHash<std::string>& resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& filter,
    float seam_angle);

}
