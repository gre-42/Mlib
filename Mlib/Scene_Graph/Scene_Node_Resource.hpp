#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <list>
#include <map>
#include <memory>
#include <regex>

namespace Mlib {

template <class TData>
struct OffsetAndQuaternion;
template <class TData, size_t tndim>
struct PointsAndAdjacency;

struct AnimatedColoredVertexArrays;
class ColoredVertexArray;
class Scene;
class SceneNode;
enum class WayPointLocation;
enum class AggregateMode;
struct SpawnPoint;

struct SceneNodeResourceFilter {
    size_t min_num = 0;
    size_t max_num = SIZE_MAX;
    std::regex regex{""};
    inline bool matches(size_t num, const std::string& name) const {
        return (num >= min_num) && (num <= max_num) && (std::regex_search(name, regex));
    }
};

class SceneNodeResource {
public:
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const {
        throw std::runtime_error("instantiate_renderable not implemented");
    }
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const {
        throw std::runtime_error("get_animated_arrays not implemented");
    }
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) {
        throw std::runtime_error("generate_triangle_rays not implemented");
    }
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
        throw std::runtime_error("generate_ray not implemented");
    }
    virtual AggregateMode aggregate_mode() const {
        throw std::runtime_error("aggregate_mode not implemented");
    }
    virtual std::list<SpawnPoint> spawn_points() const {
        throw std::runtime_error("spawn_points not implemented");
    }
    virtual std::map<WayPointLocation, PointsAndAdjacency<float, 2>> way_points() const {
        throw std::runtime_error("way_points not implemented");
    }
    virtual void set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float>>& poses) {
        throw std::runtime_error("set_relative_joint_poses not implemented");
    }
    virtual void downsample(size_t factor) {
        throw std::runtime_error("downsample not implemented");
    }
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance)
    {
        throw std::runtime_error("import_bone_weights not implemented");
    }
};

}
