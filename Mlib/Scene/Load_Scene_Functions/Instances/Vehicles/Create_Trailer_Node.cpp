#include "Create_Trailer_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
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
    CreateTrailerNode(args.renderable_scene()).execute(args);
};

CreateTrailerNode::CreateTrailerNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateTrailerNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(args.arguments.at<std::string>(KnownArgs::car_instance)).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    auto trailer_asset_id = args.arguments.at<std::string>(KnownArgs::trailer_asset_id);
    const auto& vars = args
        .asset_references
        .get_replacement_parameters("vehicles")
        .at(trailer_asset_id);
    auto node = std::make_unique<SceneNode>();
    auto pose0 = TransformationMatrix<float, double, 3>(
        fixed_identity_array<float, 3>(),
        vars.globals.at<FixedArray<double, 3>>("TRAILER_HITCH_POSITION_FEMALE"));
    auto pose1 = rb->rbi_.rbp_.abs_transformation() * pose0;
    node->set_relative_pose(pose1.t(), matrix_2_tait_bryan_angles(pose1.R()), pose1.get_scale());
    scene.add_root_node(
        args.arguments.at<std::string>(KnownArgs::trailer_node),
        std::move(node));
}
