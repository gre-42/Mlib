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
        std::lock_guard lock{max_distance_to_camera_mutex_};
        if (std::isnan(max_distance_to_camera_)) {
            double max_distance_to_camera = 0.f;
            auto add_distance = [&max_distance_to_camera](const std::string& name, double distance){
                if (!std::isfinite(distance)) {
                    throw std::runtime_error("Resource \"" + name + "\" contains non-finite maximum center distance");
                }
                max_distance_to_camera = std::max(distance, max_distance_to_camera);
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
            if (max_distance_to_camera == 0.f) {
                throw std::runtime_error("Max distance to camera is zero");
            }
            max_distance_to_camera_ = max_distance_to_camera;
        }
    }
    return max_distance_to_camera_;
}

TerrainStyleDistancesToBdry TerrainStyle::distances_to_bdry() const {
    if (std::isnan(distances_to_bdry_.min_distance_to_bdry) !=
        std::isnan(distances_to_bdry_.max_distance_to_bdry))
    {
        throw std::runtime_error("Inconsistent distance NAN-ness");
    }
    if (std::isnan(distances_to_bdry_.min_distance_to_bdry)) {
        std::lock_guard lock{distances_to_bdry_mutex_};
        if (std::isnan(distances_to_bdry_.min_distance_to_bdry)) {
            TerrainStyleDistancesToBdry distances_to_bdry{
                .min_distance_to_bdry = INFINITY,
                .max_distance_to_bdry = 0.f,
                .is_active = false
            };
            auto add_min_distance = [&distances_to_bdry](const std::string& name, double distance){
                if (!std::isfinite(distance)) {
                    throw std::runtime_error("Resource \"" + name + "\" contains non-finite minimum distance to bdry");
                }
                distances_to_bdry.min_distance_to_bdry = std::min(distance, distances_to_bdry.min_distance_to_bdry);
                distances_to_bdry.is_active |= (distance != 0);
            };
            auto add_max_distance = [&distances_to_bdry](const std::string& name, double distance){
                if (distance == INFINITY) {
                    return;
                }
                if (!std::isfinite(distance)) {
                    throw std::runtime_error("Resource \"" + name + "\" contains NAN or -INFINITY maximum distance to bdry");
                }
                distances_to_bdry.max_distance_to_bdry = std::max(distance, distances_to_bdry.max_distance_to_bdry);
                distances_to_bdry.is_active = true;
            };
            auto add_recources = [&add_min_distance, &add_max_distance](const auto& resource_names){
                for (const auto& l : resource_names) {
                    add_min_distance(l.name, l.min_distance_to_bdry);
                    add_max_distance(l.name, l.max_distance_to_bdry);
                }
            };
            add_recources(config.near_resource_names_valley);
            add_recources(config.near_resource_names_mountain);
            distances_to_bdry_ = distances_to_bdry;
        }
    }
    return distances_to_bdry_;
}
