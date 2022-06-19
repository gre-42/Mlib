#pragma once
#include <cmath>
#include <vector>

namespace Mlib {

struct ParsedResourceName;
class SceneNodeResources;

struct TerrainStyleConfig {
    std::vector<ParsedResourceName> near_resource_names_valley;
    std::vector<ParsedResourceName> near_resource_names_mountain;
    double min_near_distance_to_bdry = 0.f;
    double much_near_distance = INFINITY;
    bool is_visible() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(near_resource_names_valley);
        archive(near_resource_names_mountain);
        archive(min_near_distance_to_bdry);
        archive(much_near_distance);
    }
};

class TerrainStyle {
public:
    explicit TerrainStyle(const TerrainStyleConfig &config);
    TerrainStyleConfig config;
    bool is_visible() const;
    double max_distance_to_camera(SceneNodeResources& scene_node_resources) const;
private:
    mutable double max_distance_to_camera_ = NAN;
};

}
