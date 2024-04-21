#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

class SceneNodeResources;
enum class PhysicsMaterial: uint32_t;

void make_triangles_with_opposing_normals_two_sided(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    PhysicsMaterial material_filter);

}
