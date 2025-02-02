#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct ColumnDescription {
    std::string joint_name;
    size_t pose_index0;
    size_t pose_index1;
};

struct BvhConfig {
    size_t smooth_radius = 20;
    float smooth_alpha = 0.9f;
    bool periodic = true;
    bool demean = false;
    float scale = 1;
    // https://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/BVH.html
    // v*R = v*YXZ
    FixedArray<size_t, 3> rotation_order = {(size_t)1, (size_t)0, (size_t)2};
    FixedArray<float, 4, 4> parameter_transformation = fixed_identity_array<float, 4>();
};

static const auto x_z_my = FixedArray<float, 4, 4>::init(
    1.f,  0.f, 0.f, 0.f,
    0.f,  0.f, 1.f, 0.f,
    0.f, -1.f, 0.f, 0.f,
    0.f,  0.f, 0.f, 1.f);

static const BvhConfig blender_bvh_config{
    .smooth_radius = 20,
    .smooth_alpha = 0.9f,
    .periodic = true,
    .demean = false,
    .scale = 1,
    .rotation_order = {(size_t)2, (size_t)1, (size_t)0},
    .parameter_transformation = x_z_my};

FixedArray<float, 4, 4> get_parameter_transformation(const std::string& name);

class BvhLoader {
public:
    explicit BvhLoader(
        const std::string& filename,
        const BvhConfig& cfg = blender_bvh_config);
    const Map<std::string, OffsetAndQuaternion<float, float>>& get_frame(size_t id) const;
    Map<std::string, OffsetAndQuaternion<float, float>> get_relative_interpolated_frame(float time) const;
    Map<std::string, OffsetAndQuaternion<float, float>> get_absolute_interpolated_frame(float time) const;
    float duration() const;
private:
    void smoothen();
    void compute_absolute_transformation(
        const std::string& name,
        const Map<std::string, OffsetAndQuaternion<float, float>>& relative_transformations,
        Map<std::string, OffsetAndQuaternion<float, float>>& absolute_transformations,
        size_t ncalls) const;
    std::vector<Map<std::string, OffsetAndQuaternion<float, float>>> transformed_frames_;
    Map<std::string, FixedArray<float, 3>> offsets_;
    Map<std::string, std::string> parents_;
    std::list<ColumnDescription> columns_;
    BvhConfig cfg_;
    float frame_time_;
};

}
