#include <Mlib/Compression/Compressed_File.hpp>
#include <Mlib/Geometry/Interfaces/IRace_Logic.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Resources/Kn5_File_Resource.hpp>
#include <Mlib/OpenGL/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/OpenGL/Resources/Obj_File_Resource.hpp>
#include <Mlib/OpenGL/Resources/Proctree_File_Resource.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json/Load_Mesh_Config_Json.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(double_precision);
DECLARE_ARGUMENT(config);
}


class RaceLogic: public IRaceLogic {
public:
    explicit RaceLogic(
        AssetReferences& asset_references,
        std::string asset_id)
        : asset_references_{asset_references}
        , asset_id_{std::move(asset_id)}
    {}
    ~RaceLogic() {
        if (!asset_references_["levels"].at(asset_id_).rp.database.contains("checkpoints")) {
            asset_references_["levels"].merge_into_database(
                asset_id_,
                JsonMacroArguments{{{"checkpoints", nlohmann::json()}}});
        }
    }
    void set_start_pose(
        const TransformationMatrix<SceneDir, ScenePos, 3>& pose,
        const FixedArray<SceneDir, 3>& velocity,
        const FixedArray<SceneDir, 3>& angular_velocity,
        uint32_t rank) override
    {
        nlohmann::json j{
            {"car_node_position", pose.t},
            {"car_node_angles", matrix_2_tait_bryan_angles(pose.R) / degrees},
            {"vehicle_velocity", velocity},
            {"vehicle_angular_velocity", angular_velocity}};
        asset_references_["levels"].merge_into_database(
            asset_id_,
            JsonMacroArguments{{{std::to_string(rank), std::move(j)}}});
    }
    void set_checkpoints(
        const std::vector<TransformationMatrix<SceneDir, ScenePos, 3>>& checkpoints) override
    {
        if (checkpoints.empty()) {
            throw std::runtime_error("Received no checkpoints");
        }
        const auto geographic_mapping = TransformationMatrix<double, double, 3>::identity();
        std::vector<std::vector<double>> global_checkpoints;
        global_checkpoints.reserve(checkpoints.size());
        for (const auto& c : checkpoints) {
            global_checkpoints.push_back(
                TrackElement{
                    .elapsed_seconds = NAN,
                    .transformations = {UOffsetAndTaitBryanAngles<SceneDir, ScenePos, 3>{c.R, c.t}}}
                .to_vector(geographic_mapping));
        }
        asset_references_["levels"].merge_into_database(
            asset_id_,
            JsonMacroArguments{{{"checkpoints", std::move(global_checkpoints)}}});
    }
    void set_circularity(bool is_circular) override {
        if (asset_references_["levels"].at(asset_id_).rp.database.at<bool>("if_raceway_circular") != is_circular) {
            throw std::runtime_error("Inconsistent raceway circularity in level \"" + asset_id_ + '"');
        }
    }
private:
    AssetReferences& asset_references_;
    std::string asset_id_;
};

template <class TPos>
static void execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name);
    LoadMeshConfig<TPos> load_mesh_config = load_mesh_config_from_json<TPos>(
        args.arguments.child(KnownArgs::config));
    auto filename = args.arguments.try_path_or_variable(KnownArgs::filename).local_path();
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    auto& rendering_resources = RenderingContextStack::primary_rendering_resources();
    CompressedFile compressed_file{filename};
    if (compressed_file.has_any_extension(".obj")) {
        scene_node_resources.add_resource_loader(
            name,
            [filename, load_mesh_config, &scene_node_resources](){
                return load_renderable_obj(
                    filename,
                    load_mesh_config,
                    scene_node_resources);
            });
    } else if (compressed_file.has_any_extension(".kn5", ".ini")) {
        scene_node_resources.add_resource_loader(
            name,
            [filename,
             load_mesh_config,
             &scene_node_resources,
             &rendering_resources,
             &asset_references=args.asset_references,
             name]()
            {
                RaceLogic race_logic{ asset_references, *name };
                return load_renderable_kn5(
                    filename,
                    load_mesh_config,
                    scene_node_resources,
                    &rendering_resources,
                    &race_logic);
            });
    } else if (compressed_file.has_any_extension(".mhx2")) {
        if constexpr (std::is_same_v<TPos, float>) {
            scene_node_resources.add_resource_loader(
                name,
                [filename, load_mesh_config](){
                    return std::make_shared<Mhx2FileResource>(
                        filename,
                        load_mesh_config);
                });
        } else {
            throw std::runtime_error("MHX2 does not support double precision");
        }
    } else if (compressed_file.has_any_extension(".proctree.json")) {
        if constexpr (std::is_same_v<TPos, float>) {
            scene_node_resources.add_resource_loader(
                name,
                [filename, load_mesh_config](){
                    return std::make_shared<ProctreeFileResource>(
                        filename,
                        load_mesh_config);
                });
        } else {
            throw std::runtime_error("Proctree does not support double precision");
        }
    } else {
        throw std::runtime_error("Unknown file type: \"" + filename.string() + '"');
    }
}

static void execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    if (!args.arguments.at<bool>(KnownArgs::double_precision, false)) {
        execute<float>(args);
    } else {
        execute<CompressedScenePos>(args);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "obj_resource",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                execute(args);
            }
        );
    }
} obj;

}
