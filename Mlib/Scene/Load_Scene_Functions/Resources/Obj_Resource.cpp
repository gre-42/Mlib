#include "Obj_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Interfaces/IRace_Logic.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config_Json.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Kn5_File_Resource.hpp>
#include <Mlib/Render/Resources/Mhx2_File_Resource.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(double_precision);
DECLARE_ARGUMENT(config);
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
        execute<CompressedScenePos>(args);
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
        if (!asset_references_["levels"].at(asset_id_).rp.database.contains("checkpoints")) {
            asset_references_["levels"].merge_into_database(
                asset_id_,
                JsonMacroArguments{{{"checkpoints", nlohmann::json()}}});
        }
    }
    void set_start_pose(
        const TransformationMatrix<float, ScenePos, 3>& pose,
        const FixedArray<float, 3>& velocity,
        const FixedArray<float, 3>& angular_velocity,
        unsigned int rank) override
    {
        if (rank == 0) {
            asset_references_["levels"].merge_into_database(
                asset_id_,
                JsonMacroArguments{{
                    {"car_node_position", pose.t},
                    {"car_node_angles", matrix_2_tait_bryan_angles(pose.R) / degrees},
                    {"vehicle_velocity", velocity},
                    {"vehicle_angular_velocity", angular_velocity}
                }});
        }
    }
    void set_checkpoints(
        const std::vector<TransformationMatrix<float, ScenePos, 3>>& checkpoints) override
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
                    .transformations = {OffsetAndTaitBryanAngles{c.R, c.t}}}
                .to_vector(geographic_mapping));
        }
        asset_references_["levels"].merge_into_database(
            asset_id_,
            JsonMacroArguments{{{"checkpoints", std::move(global_checkpoints)}}});
    }
    void set_circularity(bool is_circular) override {
        if (asset_references_["levels"].at(asset_id_).rp.database.at<bool>("if_raceway_circular") != is_circular) {
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
    LoadMeshConfig<TPos> load_mesh_config = load_mesh_config_from_json<TPos>(
        args.arguments.child(KnownArgs::config));
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
                RaceLogic race_logic{ asset_references, name };
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
