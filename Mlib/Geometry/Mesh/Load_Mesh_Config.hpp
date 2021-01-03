#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

enum class BlendMode;
enum class OccludedType;
enum class OccluderType;
enum class AggregateMode;
enum class TransformationMode;

struct LoadMeshConfig {
    FixedArray<float, 3> position = FixedArray<float, 3>(0);
    FixedArray<float, 3> rotation = FixedArray<float, 3>(0);
    FixedArray<float, 3> scale = FixedArray<float, 3>(1);
    bool is_small;
    BlendMode blend_mode;
    bool cull_faces;
    OccludedType occluded_type;
    OccluderType occluder_type;
    bool occluded_by_black;
    AggregateMode aggregate_mode;
    TransformationMode transformation_mode;
    bool apply_static_lighting;
    bool werror;
};

}
