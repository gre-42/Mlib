#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <vector>

namespace Mlib {

class SceneNodeResources;

enum class SizeClassification {
    SMALL,
    LARGE
};

struct TerrainStyleConfig {
    std::vector<ParsedResourceName> near_resource_names_valley_regular;
    std::vector<ParsedResourceName> near_resource_names_mountain_regular;
    std::vector<ParsedResourceName> near_resource_names_valley_dirt;
    std::vector<ParsedResourceName> near_resource_names_mountain_dirt;
    ScenePos much_near_distance = INFINITY;
    std::string foliagemap_filename;
    float foliagemap_scale = 1.f;
    std::string mudmap_filename;
    SizeClassification size_classification = SizeClassification::SMALL;
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
        archive(mudmap_filename);
        archive(size_classification);
    }
};

struct TerrainStyleDistancesToBdry {
    ScenePos min_distance_to_bdry = NAN;
    ScenePos max_distance_to_bdry = NAN;
    bool is_active = false;
};

class TerrainStyle {
public:
    explicit TerrainStyle(const TerrainStyleConfig &config);
    TerrainStyleConfig config;
    const Array<float>& foliagemap() const;
    const Array<float>& mudmap() const;
    bool is_visible() const;
    ScenePos max_distance_to_camera(const SceneNodeResources& scene_node_resources) const;
    TerrainStyleDistancesToBdry distances_to_bdry() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(config);
    }
private:
    mutable SafeAtomicRecursiveSharedMutex max_distance_to_camera_mutex_;
    mutable SafeAtomicRecursiveSharedMutex distances_to_bdry_mutex_;
    mutable ScenePos max_distance_to_camera_ = NAN;
    mutable TerrainStyleDistancesToBdry distances_to_bdry_;

    mutable SafeAtomicRecursiveSharedMutex foliagemap_mutex_;
    mutable Array<float> foliagemap_array_;

    mutable SafeAtomicRecursiveSharedMutex mudmap_mutex_;
    mutable Array<float> mudmap_array_;
};

}
