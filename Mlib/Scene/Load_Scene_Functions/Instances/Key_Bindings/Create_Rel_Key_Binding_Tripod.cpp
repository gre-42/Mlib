#include "Create_Rel_Key_Binding_Tripod.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(translation);
DECLARE_ARGUMENT(rotation_axis);
DECLARE_ARGUMENT(velocity_press);
DECLARE_ARGUMENT(velocity_repeat);
DECLARE_ARGUMENT(angular_velocity_press);
DECLARE_ARGUMENT(angular_velocity_repeat);
DECLARE_ARGUMENT(speed_cursor);
}

const std::string CreateRelKeyBindingTripod::key = "rel_key_binding_tripod";

LoadSceneJsonUserFunction CreateRelKeyBindingTripod::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateRelKeyBindingTripod(args.renderable_scene()).execute(args);
};

CreateRelKeyBindingTripod::CreateRelKeyBindingTripod(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRelKeyBindingTripod::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    key_bindings.add_relative_movable_key_binding(RelativeMovableKeyBinding{
        .id = args.arguments.at<std::string>(KnownArgs::id),
        .role = args.arguments.at<std::string>(KnownArgs::role),
        .fixed_node = nullptr,
        .dynamic_node = [&scene=scene, &sc=selected_cameras]() -> DanglingPtr<SceneNode> {
            auto name = sc.camera_node_name();
            auto cycle = sc.cycle(name);
            if (cycle.has_value() && (cycle.value() == CameraCycleType::TRIPOD)) {
                return scene.get_node(name, DP_LOC).ptr();
            }
            return nullptr;
        },
        .translation = args.arguments.at<FixedArray<double, 3>>(KnownArgs::translation, fixed_zeros<double, 3>()),
        .rotation_axis = args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation_axis, fixed_zeros<float, 3>()),
        .velocity_press = args.arguments.at<double>(KnownArgs::velocity_press, 0.) * kph,
        .velocity_repeat = args.arguments.at<double>(KnownArgs::velocity_repeat, 0.) * kph,
        .angular_velocity_press = args.arguments.at<float>(KnownArgs::angular_velocity_press, 0.f) * radians / s,
        .angular_velocity_repeat = args.arguments.at<float>(KnownArgs::angular_velocity_repeat, 0.f) * radians / s,
        .speed_cursor = args.arguments.at<float>(KnownArgs::speed_cursor)});
}
