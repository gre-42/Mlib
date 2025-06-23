#include "Create_Trailer_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(car_instance);
DECLARE_ARGUMENT(trailer_asset_id);
DECLARE_ARGUMENT(trailer_node);
}

const std::string CreateTrailerNode::key = "create_trailer_node";

LoadSceneJsonUserFunction CreateTrailerNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateTrailerNode(args.physics_scene()).execute(args);
};

CreateTrailerNode::CreateTrailerNode(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateTrailerNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& rb = get_rigid_body_vehicle(scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::car_instance), DP_LOC));
    auto trailer_asset_id = args.arguments.at<std::string>(KnownArgs::trailer_asset_id);
    const auto& vars = args
        .asset_references["vehicles"]
        .at(trailer_asset_id)
        .rp;
    auto pose0 = TranslationMatrix<ScenePos, 3>(
        rb.trailer_hitches_.get_position_male().casted<ScenePos>() -
        vars.database.at<EFixedArray<ScenePos, 3>>("trailer_hitch_position_female"));
    auto pose1 = rb.rbp_.abs_transformation() * pose0;
    auto node = make_unique_scene_node(
        pose1.t,
        matrix_2_tait_bryan_angles(pose1.R),
        pose1.get_scale());
    scene.add_root_node(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::trailer_node),
        std::move(node),
        RenderingDynamics::MOVING,
        RenderingStrategies::OBJECT);
}
