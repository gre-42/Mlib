#pragma once
#include <string>

namespace Mlib {

class SceneNodeResources;
enum class PhysicsMaterial;

void add_cleanup_mesh_modifier(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    float min_vertex_distance,
    PhysicsMaterial min_distance_material_filter,
    bool modulo_uv);

}
