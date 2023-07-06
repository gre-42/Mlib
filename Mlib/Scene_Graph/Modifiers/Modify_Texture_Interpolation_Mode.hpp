#pragma once
#include <string>

namespace Mlib {

class SceneNodeResources;
struct ColoredVertexArrayFilter;
enum class InterpolationMode;

void modify_texture_interpolation_mode(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& filter,
    InterpolationMode magnifying_interpolation_mode);

}
