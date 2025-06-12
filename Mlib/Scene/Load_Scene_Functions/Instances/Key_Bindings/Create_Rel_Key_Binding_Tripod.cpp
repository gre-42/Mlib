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
DECLARE_ARGUMENT(user_id);
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(seat);

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
: LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateRelKeyBindingTripod::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& kb = key_bindings.add_relative_movable_key_binding(std::unique_ptr<RelativeMovableKeyBinding>(new RelativeMovableKeyBinding{
        .dynamic_node = [&scene=scene, &sc=selected_cameras]() -> DanglingPtr<SceneNode> {
            auto name = sc.camera_node_name();
            auto cycle = sc.cycle(name);
            if (cycle.has_value() && (*cycle == CameraCycleType::TRIPOD)) {
                return scene.get_node(name, DP_LOC).ptr();
            }
            return nullptr;
        },
        .translation = args.arguments.at<UFixedArray<ScenePos, 3>>(KnownArgs::translation, fixed_zeros<ScenePos, 3>()),
        .rotation_axis = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation_axis, fixed_zeros<float, 3>()),
        .velocity_press = args.arguments.at<ScenePos>(KnownArgs::velocity_press, 0.) * kph,
        .velocity_repeat = args.arguments.at<ScenePos>(KnownArgs::velocity_repeat, 0.) * kph,
        .angular_velocity_press = args.arguments.at<float>(KnownArgs::angular_velocity_press, 0.f) * radians / seconds,
        .angular_velocity_repeat = args.arguments.at<float>(KnownArgs::angular_velocity_repeat, 0.f) * radians / seconds,
        .speed_cursor = args.arguments.at<float>(KnownArgs::speed_cursor),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat)},
        .cursor_movement = std::make_shared<CursorMovement>(
            args.cursor_states,
            args.key_configurations,
            args.arguments.at<std::string>(KnownArgs::id)),
        .on_destroy_key_bindings{ DestructionFunctionsRemovalTokens{ key_bindings.on_destroy, CURRENT_SOURCE_LOCATION } },
        .on_node_clear{ DestructionFunctionsRemovalTokens{ nullptr, CURRENT_SOURCE_LOCATION } },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ nullptr, CURRENT_SOURCE_LOCATION }} }));
    kb.on_destroy_key_bindings.add([&kbs = key_bindings, &kb]() {
        kbs.delete_relative_movable_key_binding(kb);
    }, CURRENT_SOURCE_LOCATION);
}
