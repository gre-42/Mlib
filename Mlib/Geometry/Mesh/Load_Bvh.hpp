#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Quaternion.hpp>
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
    FixedArray<size_t, 3> rotation_order = {1, 0, 2};
    FixedArray<float, 4, 4> parameter_transformation = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f };
};

static const BvhConfig blender_bvh_config{
    .smooth_radius = 20,
    .smooth_alpha = 0.9f,
    .periodic = true,
    .demean = false,
    .scale = 1,
    .rotation_order = {2, 1, 0},
    .parameter_transformation = {
        1.f,  0.f, 0.f, 0.f,
        0.f,  0.f, 1.f, 0.f,
        0.f, -1.f, 0.f, 0.f,
        0.f,  0.f, 0.f, 1.f}};

FixedArray<float, 4, 4> get_parameter_transformation(const std::string& name);

class BvhLoader {
public:
    explicit BvhLoader(
        const std::string& filename,
        const BvhConfig& cfg = blender_bvh_config);
    const std::map<std::string, OffsetAndQuaternion<float>>& get_frame(size_t id) const;
    std::map<std::string, OffsetAndQuaternion<float>> get_interpolated_frame(float seconds) const;
private:
    void smoothen();
    std::vector<std::map<std::string, OffsetAndQuaternion<float>>> transformed_frames_;
    std::map<std::string, FixedArray<float, 3>> offsets_;
    std::list<ColumnDescription> columns_;
    BvhConfig cfg_;
    float frame_time_;
};

}
