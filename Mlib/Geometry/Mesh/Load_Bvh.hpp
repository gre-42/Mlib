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
    bool demean = false;
    float scale = 1;
    // https://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/BVH.html
    // v*R = v*YXZ
    FixedArray<size_t, 3> rotation_order = {1, 0, 2};
    FixedArray<float, 4, 4> parameter_transformation = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
};

static const BvhConfig blender_bvh_config{
    .demean = false,
    .scale = 1,
    .rotation_order = {2, 1, 0},
    .parameter_transformation = {
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, -1, 0, 0,
        0, 0, 0, 1}};

FixedArray<float, 4, 4> get_parameter_transformation(const std::string& name);

class BvhLoader {
public:
    explicit BvhLoader(
        const std::string& filename,
        const BvhConfig& cfg = blender_bvh_config);
    std::map<std::string, FixedArray<float, 4, 4>> get_frame(size_t id) const;
    std::map<std::string, OffsetAndQuaternion<float>> get_interpolated_frame(float seconds) const;
private:
    std::vector<std::map<std::string, FixedArray<float, 2, 3>>> frames_;
    std::map<std::string, FixedArray<float, 3>> offsets_;
    std::list<ColumnDescription> columns_;
    BvhConfig cfg_;
    float frame_time_;
};

}
