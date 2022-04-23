#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Triangle_Tangent_Error_Behavior.hpp>

namespace Mlib {

enum class BlendMode;
enum class AggregateMode;
enum class TransformationMode;
enum class ExternalRenderPassType;

struct LoadMeshConfig {
    FixedArray<float, 3> position = FixedArray<float, 3>(0.f);
    FixedArray<float, 3> rotation = FixedArray<float, 3>(0.f);
    FixedArray<float, 3> scale = FixedArray<float, 3>(1.f);
    OrderableFixedArray<float, 2> distances = default_distances_hard;
    bool is_small;
    BlendMode blend_mode;
    FixedArray<float, 4> alpha_distances = default_distances;
    bool cull_faces_default;
    bool cull_faces_alpha;
    ExternalRenderPassType occluded_pass;
    ExternalRenderPassType occluder_pass;
    AggregateMode aggregate_mode;
    TransformationMode transformation_mode;
    std::string reflection_map;
    TriangleTangentErrorBehavior triangle_tangent_error_behavior = TriangleTangentErrorBehavior::RAISE;
    bool apply_static_lighting;
    bool werror;
};

}
