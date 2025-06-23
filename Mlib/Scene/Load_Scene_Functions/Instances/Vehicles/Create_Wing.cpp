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
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const float LIFT_COEFF_UNITS = N / squared(meters / seconds);
static const float ANGLE_COEFF_UNITS = N / degrees / squared(meters / seconds);
static const float DRAG_COEFF_UNITS = N / squared(meters / seconds);

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
DECLARE_ARGUMENT(trail_source);
}

namespace KnownTrailSource {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(storage);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(minimum_velocity);
}

const std::string CreateWing::key = "wing";

LoadSceneJsonUserFunction CreateWing::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateWing(args.physics_scene()).execute(args);
};

CreateWing::CreateWing(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateWing::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> vehicle_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::vehicle), DP_LOC);
    auto& vehicle_rb = get_rigid_body_vehicle(vehicle_node);
    auto position = args.arguments.at<EFixedArray<ScenePos, 3>>(KnownArgs::position) * (ScenePos)meters;
    auto rotation = args.arguments.at<EFixedArray<float, 3>>(KnownArgs::rotation) * degrees;
    size_t wing_id = args.arguments.at<size_t>(KnownArgs::wing_id);
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    Interp<float> fac{
        args.arguments.at_vector<float>(KnownArgs::fac_v, [](float v){return v * kph;}),
        args.arguments.at<std::vector<float>>(KnownArgs::fac_c),
        OutOfRangeBehavior::CLAMP};
    std::optional<TrailSource> trail_source;
    if (args.arguments.contains(KnownArgs::trail_source)) {
        auto jtrail_source = args.arguments.child(KnownArgs::trail_source);
        jtrail_source.validate(KnownTrailSource::options);
        trail_source.emplace(
            trail_renderer.get_storage(VariableAndHash{ jtrail_source.at<std::string>(KnownTrailSource::storage) }).add_trail_extender(),
            jtrail_source.at<EFixedArray<float, 3>>(KnownTrailSource::position) * meters,
            jtrail_source.at<float>(KnownTrailSource::minimum_velocity) * kph);
    }
    DanglingPtr<SceneNode> angle_of_attack_node = nullptr;
    if (args.arguments.contains(KnownArgs::angle_of_attack_node)) {
        angle_of_attack_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::angle_of_attack_node), DP_LOC).ptr();
    }
    DanglingPtr<SceneNode> brake_angle_node = nullptr;
    if (args.arguments.contains(KnownArgs::brake_angle_node)) {
        brake_angle_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::brake_angle_node), DP_LOC).ptr();
    }
    vehicle_rb.wings_.add(
        wing_id,
        std::make_unique<Wing>(
            angle_of_attack_node,
            brake_angle_node,
            TransformationMatrix<float, ScenePos, 3>{ r, position },
            fac,
            LIFT_COEFF_UNITS * args.arguments.at<float>(KnownArgs::lift_c),
            ANGLE_COEFF_UNITS * args.arguments.at<float>(KnownArgs::angle_yz),
            ANGLE_COEFF_UNITS * args.arguments.at<float>(KnownArgs::angle_zz),
            DRAG_COEFF_UNITS * args.arguments.at<EFixedArray<float, 3>>(KnownArgs::drag),
            0.f,
            0.f,
            std::move(trail_source)));
}
