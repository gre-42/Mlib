#include "Create_Wing.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Actuators/Wing.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const float LIFT_COEFF_UNITS = N / squared(meters / s);
static const float ANGLE_COEFF_UNITS = N / degrees / squared(meters / s);
static const float DRAG_COEFF_UNITS = N / squared(meters / s);

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(vehicle);
DECLARE_ARGUMENT(angle_of_attack_node);
DECLARE_ARGUMENT(brake_angle_node);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(fac_v);
DECLARE_ARGUMENT(fac_c);
DECLARE_ARGUMENT(lift_c);
DECLARE_ARGUMENT(angle_yz);
DECLARE_ARGUMENT(angle_zz);
DECLARE_ARGUMENT(drag);
DECLARE_ARGUMENT(wing_id);
}

const std::string CreateWing::key = "wing";

LoadSceneJsonUserFunction CreateWing::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateWing(args.renderable_scene()).execute(args);
};

CreateWing::CreateWing(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWing::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> vehicle_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::vehicle), DP_LOC);
    auto& vehicle_rb = get_rigid_body_vehicle(vehicle_node);
    auto position = args.arguments.at<FixedArray<double, 3>>(KnownArgs::position) * (double)meters;
    auto rotation = args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation) * degrees;
    size_t wing_id = args.arguments.at<size_t>(KnownArgs::wing_id);
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    Interp<float> fac{
        args.arguments.at_vector<float>(KnownArgs::fac_v, [](float v){return v * kph;}),
        args.arguments.at<std::vector<float>>(KnownArgs::fac_c),
        OutOfRangeBehavior::CLAMP};
    auto tp = vehicle_rb.wings_.insert({
        wing_id,
        std::make_unique<Wing>(
            TransformationMatrix<float, double, 3>{ r, position },
            fac,
            LIFT_COEFF_UNITS * args.arguments.at<float>(KnownArgs::lift_c),
            ANGLE_COEFF_UNITS * args.arguments.at<float>(KnownArgs::angle_yz),
            ANGLE_COEFF_UNITS * args.arguments.at<float>(KnownArgs::angle_zz),
            DRAG_COEFF_UNITS * args.arguments.at<FixedArray<float, 3>>(KnownArgs::drag),
            0.f,
            0.f)});
    if (!tp.second) {
        THROW_OR_ABORT("Wing with ID \"" + std::to_string(wing_id) + "\" already exists");
    }
    if (args.arguments.contains(KnownArgs::angle_of_attack_node)) {
        scene.get_node(args.arguments.at<std::string>(KnownArgs::angle_of_attack_node), DP_LOC)->set_relative_movable(
            observer_ptr<IRelativeMovable, DanglingRef<const SceneNode>>{&tp.first->second->angle_of_attack_movable, nullptr});
    }
    if (args.arguments.contains(KnownArgs::brake_angle_node)) {
        scene.get_node(args.arguments.at<std::string>(KnownArgs::brake_angle_node), DP_LOC)->set_relative_movable(
            observer_ptr<IRelativeMovable, DanglingRef<const SceneNode>>{&tp.first->second->brake_angle_movable, nullptr});
    }
}
