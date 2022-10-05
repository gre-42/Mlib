#pragma once
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <cmath>
#include <shared_mutex>
#include <vector>

namespace Mlib {

class SceneNodeResources;

struct TerrainStyleConfig {
    std::vector<ParsedResourceName> near_resource_names_valley;
    std::vector<ParsedResourceName> near_resource_names_mountain;
    double much_near_distance = INFINITY;
    bool is_visible() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(near_resource_names_valley);
        archive(near_resource_names_mountain);
        archive(much_near_distance);
    }
};

struct TerrainStyleDistancesToBdry {
    double min_distance_to_bdry = NAN;
    double max_distance_to_bdry = NAN;
    bool is_active = false;
};

class TerrainStyle {
public:
    explicit TerrainStyle(const TerrainStyleConfig &config);
    TerrainStyleConfig config;
    bool is_visible() const;
    double max_distance_to_camera(SceneNodeResources& scene_node_resources) const;
    TerrainStyleDistancesToBdry distances_to_bdry() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(config);
    }
private:
    mutable std::shared_mutex max_distance_to_camera_mutex_;
    mutable std::shared_mutex distances_to_bdry_mutex_;
    mutable double max_distance_to_camera_ = NAN;
    mutable TerrainStyleDistancesToBdry distances_to_bdry_;
};

}
