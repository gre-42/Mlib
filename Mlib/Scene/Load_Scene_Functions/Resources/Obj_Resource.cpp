#include "Obj_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Interfaces/IRace_Logic.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Kn5_File_Resource.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(center_distances);
DECLARE_ARGUMENT(max_triangle_distance);
DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(alpha_distances);
DECLARE_ARGUMENT(cull_faces_default);
DECLARE_ARGUMENT(cull_faces_alpha);
DECLARE_ARGUMENT(occluded_pass);
DECLARE_ARGUMENT(occluder_pass);
DECLARE_ARGUMENT(anisotropic_filtering_level);
DECLARE_ARGUMENT(magnifying_interpolation_mode);
DECLARE_ARGUMENT(aggregate_mode);
DECLARE_ARGUMENT(transformation_mode);
DECLARE_ARGUMENT(reflection_map);
DECLARE_ARGUMENT(emissive_factor);
DECLARE_ARGUMENT(ambient_factor);
DECLARE_ARGUMENT(diffuse_factor);
DECLARE_ARGUMENT(specular_factor);
DECLARE_ARGUMENT(fresnel_ambient);
DECLARE_ARGUMENT(fresnel_min);
DECLARE_ARGUMENT(fresnel_max);
DECLARE_ARGUMENT(fresnel_exponent);
DECLARE_ARGUMENT(desaturate);
DECLARE_ARGUMENT(histogram);
DECLARE_ARGUMENT(lighten);
DECLARE_ARGUMENT(triangle_tangent_error_behavior);
DECLARE_ARGUMENT(rectangle_triangulation_mode);
DECLARE_ARGUMENT(dynamically_lighted);
DECLARE_ARGUMENT(physics_material);
DECLARE_ARGUMENT(double_precision);
DECLARE_ARGUMENT(werror);
}

const std::string ObjResource::key = "obj_resource";

LoadSceneJsonUserFunction ObjResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void ObjResource::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (!args.arguments.at<bool>(KnownArgs::double_precision, false)) {
        execute<float>(args);
    } else {
        execute<double>(args);
    }
}

class RaceLogic: public IRaceLogic {
public:
    explicit RaceLogic(
        AssetReferences& asset_references,
        std::string asset_id)
    : asset_references_{asset_references},
      asset_id_{std::move(asset_id)}
    {}
    ~RaceLogic() {
        if (!asset_references_["levels"].at(asset_id_).rp.database.contains("CHECKPOINTS")) {
            asset_references_["levels"].merge_into_database(
                asset_id_,
                JsonMacroArguments{{{"CHECKPOINTS", nlohmann::json()}}});
        }
    }
    void set_start_pose(
        const TransformationMatrix<float, double, 3>& pose,
        unsigned int rank) override
    {
        if (rank == 0) {
            asset_references_["levels"].merge_into_database(
                asset_id_,
                JsonMacroArguments{{
                    {"CAR_NODE_POSITION", pose.t()},
                    {"CAR_NODE_ANGLES", matrix_2_tait_bryan_angles(pose.R()) / degrees}
                }});
        }
    }
    void set_checkpoints(
        const std::vector<TransformationMatrix<float, double, 3>>& checkpoints) override
    {
        if (checkpoints.empty()) {
            THROW_OR_ABORT("Received no checkpoints");
        }
        const auto geographic_mapping = TransformationMatrix<double, double, 3>::identity();
        std::vector<std::vector<double>> global_checkpoints;
        global_checkpoints.reserve(checkpoints.size());
        for (const auto& c : checkpoints) {
            global_checkpoints.push_back(
                TrackElement{
                    .elapsed_seconds = NAN,
                    .transformations = {OffsetAndTaitBryanAngles{c.R(), c.t()}}}
                .to_vector(geographic_mapping));
        }
        asset_references_["levels"].merge_into_database(
            asset_id_,
            JsonMacroArguments{{{"CHECKPOINTS", std::move(global_checkpoints)}}});
    }
    void set_circularity(bool is_circular) override {
        if (asset_references_["levels"].at(asset_id_).rp.database.at<bool>("IF_RACEWAY_CIRCULAR") != is_circular) {
            THROW_OR_ABORT("Inconsistent raceway circularity in level \"" + asset_id_ + '"');
        }
    }
private:
    AssetReferences& asset_references_;
    std::string asset_id_;
};

template <class TPos>
void ObjResource::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto name = args.arguments.at<std::string>(KnownArgs::name);
    LoadMeshConfig<TPos> load_mesh_config{
        .position = args.arguments.at<FixedArray<TPos, 3>>(KnownArgs::position) * (TPos)meters,
        .rotation = args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation) * degrees,
        .scale = args.arguments.at<FixedArray<float, 3>>(KnownArgs::scale),
        .center_distances = OrderableFixedArray<float, 2>{
            args.arguments.at<FixedArray<float, 2>>(
                KnownArgs::center_distances,
                FixedArray<float, 2>{0.f, INFINITY }) * meters},
        .max_triangle_distance = args.arguments.at<float>(KnownArgs::max_triangle_distance, INFINITY) * meters,
        .blend_mode = blend_mode_from_string(args.arguments.at<std::string>(KnownArgs::blend_mode)),
        .alpha_distances = args.arguments.at<OrderableFixedArray<float, 4>>(KnownArgs::alpha_distances),
        .cull_faces_default = args.arguments.at<bool>(KnownArgs::cull_faces_default, true),
        .cull_faces_alpha = args.arguments.at<bool>(KnownArgs::cull_faces_alpha, true),
        .occluded_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluded_pass)),
        .occluder_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluder_pass)),
        .anisotropic_filtering_level = args.arguments.at<unsigned int>(KnownArgs::anisotropic_filtering_level, 0),
        .magnifying_interpolation_mode = interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::magnifying_interpolation_mode, "nearest")),
        .aggregate_mode = aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::aggregate_mode)),
        .transformation_mode = transformation_mode_from_string(args.arguments.at<std::string>(KnownArgs::transformation_mode)),
        .reflection_map = args.arguments.at<std::string>(KnownArgs::reflection_map, ""),
        .emissive_factor = args.arguments.at<FixedArray<float, 3>>(KnownArgs::emissive_factor, fixed_ones<float, 3>()),
        .ambient_factor = args.arguments.at<FixedArray<float, 3>>(KnownArgs::ambient_factor, fixed_ones<float, 3>()),
        .diffuse_factor = args.arguments.at<FixedArray<float, 3>>(KnownArgs::diffuse_factor, fixed_ones<float, 3>()),
        .specular_factor = args.arguments.at<FixedArray<float, 3>>(KnownArgs::specular_factor, fixed_ones<float, 3>()),
        .fresnel{
            .reflectance = {
                .min = args.arguments.at<float>(KnownArgs::fresnel_min, 0.f),
                .max = args.arguments.at<float>(KnownArgs::fresnel_max, 0.f),
                .exponent = args.arguments.at<float>(KnownArgs::fresnel_exponent, 0.f)
            },
            .ambient = OrderableFixedArray{args.arguments.at<FixedArray<float, 3>>(KnownArgs::fresnel_ambient, fixed_zeros<float, 3>())}
        },
        .desaturate = args.arguments.at<float>(KnownArgs::desaturate, 0.f),
        .histogram = args.arguments.try_path_or_variable(KnownArgs::histogram).path,
        .lighten = args.arguments.at<FixedArray<float, 3>>(KnownArgs::lighten, fixed_zeros<float, 3>()),
        .triangle_tangent_error_behavior = args.arguments.contains(KnownArgs::triangle_tangent_error_behavior)
            ? triangle_tangent_error_behavior_from_string(args.arguments.at<std::string>(KnownArgs::triangle_tangent_error_behavior))
            : TriangleTangentErrorBehavior::RAISE,
        .apply_static_lighting = false,
        .laplace_ao_strength = 0.f,
        .dynamically_lighted = args.arguments.at<bool>(KnownArgs::dynamically_lighted, false),
        .physics_material = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::physics_material, "attr_visible|attr_collide")),
        .rectangle_triangulation_mode = rectangle_triangulation_mode_from_string(args.arguments.at<std::string>(KnownArgs::rectangle_triangulation_mode, "delaunay")),
        .werror = args.arguments.at<bool>(KnownArgs::werror, true)};
    std::string filename = args.arguments.try_path_or_variable(KnownArgs::filename).path;
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    auto& rendering_resources = RenderingContextStack::primary_rendering_resources();
    if (filename.ends_with(".obj")) {
        scene_node_resources.add_resource_loader(
            name,
            [filename, load_mesh_config, &scene_node_resources](){
                return load_renderable_obj(
                    filename,
                    load_mesh_config,
                    scene_node_resources);
            });
    } else if (filename.ends_with(".kn5") || filename.ends_with(".ini")) {
        scene_node_resources.add_resource_loader(
            name,
            [filename,
             load_mesh_config,
             &scene_node_resources,
             &rendering_resources,
             &asset_references=args.asset_references,
             name]()
            {
                RaceLogic race_logic{asset_references, name};
                return load_renderable_kn5(
                    filename,
                    load_mesh_config,
                    scene_node_resources,
                    &rendering_resources,
                    &race_logic);
            });
    } else if (filename.ends_with(".mhx2")) {
        if constexpr (std::is_same_v<TPos, float>) {
            scene_node_resources.add_resource_loader(
                name,
                [filename, load_mesh_config](){
                    return std::make_shared<Mhx2FileResource>(
                        filename,
                        load_mesh_config);
                });
        } else {
            THROW_OR_ABORT("MHX2 does not support double precision");
        }
    } else {
        THROW_OR_ABORT("Unknown file type: " + filename);
    }
}
