#include "Terrain_Style.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

bool TerrainStyleConfig::is_visible() const {
    return (!near_resource_names_valley_regular.empty() ||
            !near_resource_names_mountain_regular.empty() ||
            !near_resource_names_valley_dirt.empty() ||
            !near_resource_names_mountain_dirt.empty()) &&
            (much_near_distance != INFINITY);
}

TerrainStyle::TerrainStyle(const TerrainStyleConfig &config)
    : config{ config }
{}

bool TerrainStyle::is_visible() const {
    return config.is_visible();
}

ScenePos TerrainStyle::max_distance_to_camera(const SceneNodeResources& scene_node_resources) const {
    {
        std::shared_lock lock{max_distance_to_camera_mutex_};
        if (!std::isnan(max_distance_to_camera_)) {
            return max_distance_to_camera_;
        }
    }
    std::scoped_lock lock{max_distance_to_camera_mutex_};
    if (std::isnan(max_distance_to_camera_)) {
        ScenePos max_distance_to_camera2 = 0.f;
        auto add_distance = [&max_distance_to_camera2](const std::string& name, ScenePos distance2){
            if (!std::isfinite(distance2)) {
                THROW_OR_ABORT("Resource \"" + name + "\" contains non-finite maximum center distance");
            }
            max_distance_to_camera2 = std::max(distance2, max_distance_to_camera2);
        };
        auto add_cvas = [&add_distance](const auto& cvas, const std::string& name, BillboardId billboard_id){
            for (const auto& cva : cvas) {
                if (billboard_id != BILLBOARD_ID_NONE) {
                    add_distance(name, cva->material.billboard_atlas_instance(billboard_id, name).max_center_distance2);
                } else {
                    add_distance(name, cva->morphology.center_distances2(1));
                }
            }
        };
        auto add_recources = [&scene_node_resources, &add_cvas](const auto& resource_names){
            for (const auto& l : resource_names) {
                add_cvas(scene_node_resources.get_arrays(*l.name, ColoredVertexArrayFilter{})->dcvas, *l.name, l.billboard_id);
                add_cvas(scene_node_resources.get_arrays(*l.name, ColoredVertexArrayFilter{})->scvas, *l.name, l.billboard_id);
            }
        };
        add_recources(config.near_resource_names_valley_regular);
        add_recources(config.near_resource_names_mountain_regular);
        add_recources(config.near_resource_names_valley_dirt);
        add_recources(config.near_resource_names_mountain_dirt);
        if (max_distance_to_camera2 == 0.f) {
            THROW_OR_ABORT("Max distance to camera is zero");
        }
        max_distance_to_camera_ = std::sqrt(max_distance_to_camera2);
    }
    return max_distance_to_camera_;
}

TerrainStyleDistancesToBdry TerrainStyle::distances_to_bdry() const {
    {
        std::shared_lock lock{distances_to_bdry_mutex_};
        if (std::isnan(distances_to_bdry_.min_distance_to_bdry) !=
            std::isnan(distances_to_bdry_.max_distance_to_bdry))
        {
            THROW_OR_ABORT("Inconsistent distance NAN-ness (0)");
        }
        if (!std::isnan(distances_to_bdry_.min_distance_to_bdry)) {
            return distances_to_bdry_;
        }
    }
    std::scoped_lock lock{distances_to_bdry_mutex_};
    if (std::isnan(distances_to_bdry_.min_distance_to_bdry) !=
        std::isnan(distances_to_bdry_.max_distance_to_bdry))
    {
        THROW_OR_ABORT("Inconsistent distance NAN-ness (1)");
    }
    if (std::isnan(distances_to_bdry_.min_distance_to_bdry)) {
        TerrainStyleDistancesToBdry distances_to_bdry{
            .min_distance_to_bdry = INFINITY,
            .max_distance_to_bdry = 0.f,
            .is_active = false
        };
        auto add_min_distance = [&distances_to_bdry](const std::string& name, ScenePos distance){
            if (!std::isfinite(distance)) {
                THROW_OR_ABORT("Resource \"" + name + "\" contains non-finite minimum distance to bdry");
            }
            distances_to_bdry.min_distance_to_bdry = std::min(distance, distances_to_bdry.min_distance_to_bdry);
            distances_to_bdry.is_active |= (distance != 0);
        };
        auto add_max_distance = [&distances_to_bdry](const std::string& name, ScenePos distance){
            if (distance == INFINITY) {
                return;
            }
            if (!std::isfinite(distance)) {
                THROW_OR_ABORT("Resource \"" + name + "\" contains NAN or -INFINITY maximum distance to bdry");
            }
            distances_to_bdry.max_distance_to_bdry = std::max(distance, distances_to_bdry.max_distance_to_bdry);
            distances_to_bdry.is_active = true;
        };
        auto add_recources = [&add_min_distance, &add_max_distance](const auto& resource_names){
            for (const auto& l : resource_names) {
                add_min_distance(*l.name, l.min_distance_to_bdry);
                add_max_distance(*l.name, l.max_distance_to_bdry);
            }
        };
        add_recources(config.near_resource_names_valley_regular);
        add_recources(config.near_resource_names_mountain_regular);
        add_recources(config.near_resource_names_valley_dirt);
        add_recources(config.near_resource_names_mountain_dirt);
        distances_to_bdry_ = distances_to_bdry;
    }
    return distances_to_bdry_;
}

const Array<float>& TerrainStyle::foliagemap() const {
    if (config.foliagemap_filename.empty()) {
        return foliagemap_array_;
    }
    {
        std::shared_lock lock{foliagemap_mutex_};
        if (foliagemap_array_.initialized()) {
            return foliagemap_array_;
        }
    }
    {
        std::scoped_lock lock{foliagemap_mutex_};
        if (foliagemap_array_.initialized()) {
            return foliagemap_array_;
        }
        foliagemap_array_ = StbImage1::load_from_file(config.foliagemap_filename).to_float_grayscale();
    }
    return foliagemap_array_;
}

const Array<float>& TerrainStyle::mudmap() const {
    if (config.mudmap_filename.empty()) {
        return mudmap_array_;
    }
    {
        std::shared_lock lock{mudmap_mutex_};
        if (mudmap_array_.initialized()) {
            return mudmap_array_;
        }
    }
    {
        std::scoped_lock lock{mudmap_mutex_};
        if (mudmap_array_.initialized()) {
            return mudmap_array_;
        }
        mudmap_array_ = StbImage1::load_from_file(config.mudmap_filename).to_float_grayscale();
    }
    return mudmap_array_;
}
