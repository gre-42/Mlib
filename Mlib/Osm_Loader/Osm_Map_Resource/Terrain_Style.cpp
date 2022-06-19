#include "Terrain_Style.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

bool TerrainStyleConfig::is_visible() const {
    return (!near_resource_names_valley.empty() ||
            !near_resource_names_mountain.empty()) &&
            (much_near_distance != INFINITY);
}

TerrainStyle::TerrainStyle(const TerrainStyleConfig &config)
: config{config}
{}

bool TerrainStyle::is_visible() const {
    return config.is_visible();
}

double TerrainStyle::max_distance_to_camera(SceneNodeResources& scene_node_resources) const {
    if (std::isnan(max_distance_to_camera_)) {
        max_distance_to_camera_ = 0.f;
        auto add_distance = [this](const std::string& name, double distance){
            if (!std::isfinite(distance)) {
                throw std::runtime_error("Resource \"" + name + "\" contains non-finite maximum center distance");
            }
            max_distance_to_camera_ = std::max(distance, max_distance_to_camera_);
        };
        auto add_cvas = [&add_distance](const auto& cvas, const std::string& name, uint32_t billboard_id){
            for (const auto& cva : cvas) {
                if (billboard_id != UINT32_MAX) {
                    add_distance(name, cva->material.billboard_atlas_instance(billboard_id).max_center_distance);
                } else {
                    add_distance(name, cva->material.center_distances(1));
                }
            }
        };
        auto add_recources = [&scene_node_resources, &add_cvas](const auto& resource_names){
            for (const auto& l : resource_names) {
                add_cvas(scene_node_resources.get_animated_arrays(l.name)->dcvas, l.name, l.billboard_id);
                add_cvas(scene_node_resources.get_animated_arrays(l.name)->scvas, l.name, l.billboard_id);
            }
        };
        add_recources(config.near_resource_names_valley);
        add_recources(config.near_resource_names_mountain);
        if (max_distance_to_camera_ == 0.f) {
            throw std::runtime_error("Max distance to camera is zero");
        }
    }
    return max_distance_to_camera_;
}
