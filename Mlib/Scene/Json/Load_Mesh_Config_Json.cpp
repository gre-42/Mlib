#include "Load_Mesh_Config_Json.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Delaunay_Error_Behavior.hpp>
#include <Mlib/Geometry/Interfaces/IRace_Logic.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance_Json.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json/Blend_Map_Texture_Json.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(center_distances);
DECLARE_ARGUMENT(max_triangle_distance);
DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(alpha_distances);
DECLARE_ARGUMENT(fog_distances);
DECLARE_ARGUMENT(fog_ambient);
DECLARE_ARGUMENT(cull_faces_default);
DECLARE_ARGUMENT(cull_faces_alpha);
DECLARE_ARGUMENT(occluded_pass);
DECLARE_ARGUMENT(occluder_pass);
DECLARE_ARGUMENT(anisotropic_filtering_level);
DECLARE_ARGUMENT(magnifying_interpolation_mode);
DECLARE_ARGUMENT(aggregate_mode);
DECLARE_ARGUMENT(transformation_mode);
DECLARE_ARGUMENT(billboards);
DECLARE_ARGUMENT(reflection_map);
DECLARE_ARGUMENT(emissive_factor);
DECLARE_ARGUMENT(ambient_factor);
DECLARE_ARGUMENT(diffuse_factor);
DECLARE_ARGUMENT(specular_factor);
DECLARE_ARGUMENT(reflectance);
DECLARE_ARGUMENT(fresnel);
DECLARE_ARGUMENT(desaturate);
DECLARE_ARGUMENT(histogram);
DECLARE_ARGUMENT(lighten);
DECLARE_ARGUMENT(textures);
DECLARE_ARGUMENT(period_world);
DECLARE_ARGUMENT(triangle_tangent_error_behavior);
DECLARE_ARGUMENT(rectangle_triangulation_mode);
DECLARE_ARGUMENT(delaunay_error_behavior);
DECLARE_ARGUMENT(dynamically_lighted);
DECLARE_ARGUMENT(physics_material);
DECLARE_ARGUMENT(werror);
}

using namespace Mlib;

template <class TPos>
LoadMeshConfig<TPos> Mlib::load_mesh_config_from_json(const JsonMacroArguments& j)
{
    using I = funpack_t<TPos>;
    j.validate(KnownArgs::options);
    return LoadMeshConfig<TPos>{
        .position = (j.at<UFixedArray<I, 3>>(KnownArgs::position) * (I)meters).template casted<TPos>(),
        .rotation = j.at<UFixedArray<float, 3>>(KnownArgs::rotation) * degrees,
        .scale = j.at<UFixedArray<float, 3>>(KnownArgs::scale),
        .center_distances = OrderableFixedArray<float, 2>{
            j.at<UFixedArray<float, 2>>(
                KnownArgs::center_distances,
                default_step_distances) * meters},
        .max_triangle_distance = j.at<float>(KnownArgs::max_triangle_distance, INFINITY) * meters,
        .blend_mode = blend_mode_from_string(j.at<std::string>(KnownArgs::blend_mode)),
        .alpha_distances = j.at<UOrderableFixedArray<float, 4>>(KnownArgs::alpha_distances),
        .fog_distances = j.at<UOrderableFixedArray<float, 2>>(KnownArgs::fog_distances, default_step_distances),
        .fog_ambient = j.at<UOrderableFixedArray<float, 3>>(KnownArgs::fog_ambient, OrderableFixedArray<float, 3>(-1.f)),
        .cull_faces_default = j.at<bool>(KnownArgs::cull_faces_default, true),
        .cull_faces_alpha = j.at<bool>(KnownArgs::cull_faces_alpha, true),
        .occluded_pass = external_render_pass_type_from_string(j.at<std::string>(KnownArgs::occluded_pass)),
        .occluder_pass = external_render_pass_type_from_string(j.at<std::string>(KnownArgs::occluder_pass)),
        .anisotropic_filtering_level = j.at<unsigned int>(KnownArgs::anisotropic_filtering_level, 0),
        .magnifying_interpolation_mode = interpolation_mode_from_string(j.at<std::string>(KnownArgs::magnifying_interpolation_mode, "nearest")),
        .aggregate_mode = aggregate_mode_from_string(j.at<std::string>(KnownArgs::aggregate_mode)),
        .transformation_mode = transformation_mode_from_string(j.at<std::string>(KnownArgs::transformation_mode)),
        .billboard_atlas_instances = j.at<std::vector<BillboardAtlasInstance>>(KnownArgs::billboards, {}),
        .reflection_map = VariableAndHash{ j.at_non_null<std::string>(KnownArgs::reflection_map, "") },
        .emissive_factor = j.at<UFixedArray<float, 3>>(KnownArgs::emissive_factor, fixed_ones<float, 3>()),
        .ambient_factor = j.at<UFixedArray<float, 3>>(KnownArgs::ambient_factor, fixed_ones<float, 3>()),
        .diffuse_factor = j.at<UFixedArray<float, 3>>(KnownArgs::diffuse_factor, fixed_ones<float, 3>()),
        .specular_factor = j.at<UFixedArray<float, 3>>(KnownArgs::specular_factor, fixed_ones<float, 3>()),
        .reflectance = j.at<UFixedArray<float, 3>>(KnownArgs::reflectance, fixed_zeros<float, 3>()),
        .fresnel = j.at<FresnelAndAmbient>(KnownArgs::fresnel, FresnelAndAmbient{}),
        .desaturate = j.at<float>(KnownArgs::desaturate, 0.f),
        .histogram = j.try_path_or_variable(KnownArgs::histogram).path,
        .lighten = j.at<UFixedArray<float, 3>>(KnownArgs::lighten, fixed_zeros<float, 3>()),
        .textures = blend_map_textures_from_json(j, KnownArgs::textures),
        .period_world = j.contains(KnownArgs::period_world)
            ? j.at<float>(KnownArgs::period_world)
            : 0.f,
        .triangle_tangent_error_behavior = j.contains(KnownArgs::triangle_tangent_error_behavior)
            ? triangle_tangent_error_behavior_from_string(j.at<std::string>(KnownArgs::triangle_tangent_error_behavior))
            : TriangleTangentErrorBehavior::THROW,
        .apply_static_lighting = false,
        .laplace_ao_strength = 0.f,
        .dynamically_lighted = j.at<bool>(KnownArgs::dynamically_lighted, false),
        .physics_material = physics_material_from_string(j.at<std::string>(KnownArgs::physics_material, "attr_visible|attr_collide")),
        .rectangle_triangulation_mode = rectangle_triangulation_mode_from_string(j.at<std::string>(KnownArgs::rectangle_triangulation_mode, "delaunay")),
        .delaunay_error_behavior = delaunay_error_behavior_from_string(j.at<std::string>(KnownArgs::delaunay_error_behavior, "throw")),
        .werror = j.at<bool>(KnownArgs::werror, true)};
}

namespace Mlib {

template LoadMeshConfig<float> Mlib::load_mesh_config_from_json<float>(const JsonMacroArguments& j);
template LoadMeshConfig<CompressedScenePos> Mlib::load_mesh_config_from_json<CompressedScenePos>(const JsonMacroArguments& j);

}
