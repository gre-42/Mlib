#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Triangle_Tangent_Error_Behavior.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <vector>

namespace Mlib {

enum class BlendMode;
enum class AggregateMode;
enum class TransformationMode;
enum class ExternalRenderPassType;
enum class PhysicsMaterial: uint32_t;
enum class RectangleTriangulationMode;

template <class TPos>
struct LoadMeshConfig {
    FixedArray<TPos, 3> position = FixedArray<TPos, 3>((TPos)0.f);
    FixedArray<float, 3> rotation = FixedArray<float, 3>(0.f);
    FixedArray<float, 3> scale = FixedArray<float, 3>(1.f);
    OrderableFixedArray<float, 2> center_distances = default_step_distances;
    float max_triangle_distance = INFINITY;
    BlendMode blend_mode;
    FixedArray<float, 4> alpha_distances = default_linear_distances;
    FixedArray<float, 2> fog_distances = default_step_distances;
    FixedArray<float, 3> fog_ambient = { -1.f, -1.f, -1.f };
    bool cull_faces_default;
    bool cull_faces_alpha;
    ExternalRenderPassType occluded_pass;
    ExternalRenderPassType occluder_pass;
    unsigned int anisotropic_filtering_level = 0;
    InterpolationMode magnifying_interpolation_mode;
    AggregateMode aggregate_mode;
    TransformationMode transformation_mode;
    VariableAndHash<std::string> reflection_map;
    FixedArray<float, 3> emissive_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> ambient_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> diffuse_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> specular_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> reflectance = FixedArray<float, 3>(0.f);
    FresnelAndAmbient fresnel{
        .reflectance = {
            .min = 0.f,
            .max = 0.f,
            .exponent = 0.f
        },
        .ambient = {0.f, 0.f, 0.f}
    };
    float desaturate = 0.f;
    std::string histogram;
    FixedArray<float, 3> lighten = FixedArray<float, 3>(0.f);
    std::vector<BlendMapTexture> textures;
    float period_world;
    TriangleTangentErrorBehavior triangle_tangent_error_behavior = TriangleTangentErrorBehavior::THROW;
    bool apply_static_lighting;
    float laplace_ao_strength;
    bool dynamically_lighted;
    PhysicsMaterial physics_material;
    RectangleTriangulationMode rectangle_triangulation_mode;
    bool werror;
};

}
