#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <list>
#include <map>
#include <memory>
#include <regex>

namespace Mlib {

template <class TData>
class OffsetAndQuaternion;
template <class TData, size_t tndim>
struct PointsAndAdjacency;
template <class TData, size_t n>
class TransformationMatrix;

struct AnimatedColoredVertexArrays;
struct ColoredVertexArray;
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
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const;
    virtual TransformationMatrix<double, 3> get_geographic_mapping(SceneNode& scene_node) const;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false);
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to);
    virtual AggregateMode aggregate_mode() const;
    virtual std::list<SpawnPoint> spawn_points() const;
    virtual std::map<WayPointLocation, PointsAndAdjacency<float, 2>> way_points() const;
    virtual void set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float>>& poses);
    virtual void downsample(size_t factor);
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance);
};

}
