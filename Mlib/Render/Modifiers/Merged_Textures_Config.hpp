#pragma once
#include <string>

namespace Mlib {

enum class BlendMode;
enum class AggregateMode;

struct MergedTexturesConfig {
    std::string resource_name;
    std::string array_name;
    std::string texture_name;
    BlendMode blend_mode;
    AggregateMode aggregate_mode;
    float max_triangle_distance;
    bool cull_faces;
};

}
