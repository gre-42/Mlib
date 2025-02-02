#pragma once
#include <string>

namespace Mlib {

class SceneNodeResources;
struct ColoredVertexArrayFilter;

void add_shade_auto_modifier(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& filter,
    float seam_angle);

}
