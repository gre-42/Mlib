#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <shared_mutex>
#include <vector>

namespace Mlib {

class SceneNodeResources;

struct TerrainStyleConfig {
    std::vector<ParsedResourceName> near_resource_names_valley_regular;
    std::vector<ParsedResourceName> near_resource_names_mountain_regular;
    std::vector<ParsedResourceName> near_resource_names_valley_dirt;
    std::vector<ParsedResourceName> near_resource_names_mountain_dirt;
    double much_near_distance = INFINITY;
    std::string foliagemap_filename;
    float foliagemap_scale = 1.f;
    bool is_visible() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(near_resource_names_valley_regular);
        archive(near_resource_names_mountain_regular);
        archive(near_resource_names_valley_dirt);
        archive(near_resource_names_mountain_dirt);
        archive(much_near_distance);
        archive(foliagemap_filename);
        archive(foliagemap_scale);
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
    const Array<float>& foliagemap() const;
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

    mutable std::shared_mutex foliagemap_mutex_;
    mutable Array<float> foliagemap_array_;
};

}
