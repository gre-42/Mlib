#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Delaunay_Error_Behavior.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Material/Shading.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
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
struct BillboardAtlasInstance;

template <class TPos>
struct LoadMeshConfig {
    FixedArray<TPos, 3> position = FixedArray<TPos, 3>((TPos)0.f);
    FixedArray<float, 3> rotation = FixedArray<float, 3>(0.f);
    FixedArray<float, 3> scale = FixedArray<float, 3>(1.f);
    OrderableFixedArray<float, 2> center_distances = default_step_distances;
    float max_triangle_distance = INFINITY;
    BlendMode blend_mode;
    FixedArray<float, 4> alpha_distances = default_linear_distances;
    bool cull_faces_default;
    bool cull_faces_alpha;
    ExternalRenderPassType occluded_pass;
    ExternalRenderPassType occluder_pass;
    unsigned int anisotropic_filtering_level = 0;
    MipmapMode mipmap_mode = MipmapMode::NO_MIPMAPS;
    InterpolationMode magnifying_interpolation_mode = InterpolationMode::NEAREST;
    AggregateMode aggregate_mode;
    TransformationMode transformation_mode;
    std::vector<BillboardAtlasInstance> billboard_atlas_instances;
    VariableAndHash<std::string> reflection_map;
    Shading shading;
    FixedArray<float, 3> emissive_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> ambient_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> diffuse_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> specular_factor = FixedArray<float, 3>(1.f);
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
    RectangleTriangulationMode rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY;
    DelaunayErrorBehavior delaunay_error_behavior = DelaunayErrorBehavior::THROW;
    bool werror;
};

}
