#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class BlendMode;
enum class ExternalRenderPassType;
enum class ExternalRenderPassType;
enum class AggregateMode;

struct MergedTexturesConfig {
    std::string resource_name;
    std::string array_name;
    std::string texture_name;
    BlendMode blend_mode;
    int continuous_blending_z_order;
    ExternalRenderPassType occluded_pass;
    ExternalRenderPassType occluder_pass;
    AggregateMode aggregate_mode;
    float max_triangle_distance;
    bool cull_faces;
    FixedArray<float, 3> ambience;
};

}
