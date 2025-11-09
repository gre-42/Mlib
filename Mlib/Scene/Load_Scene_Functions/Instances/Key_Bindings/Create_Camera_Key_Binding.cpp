#include "Create_Camera_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(tpe);
}

CreateCameraKeyBinding::CreateCameraKeyBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateCameraKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    key_bindings.add_camera_key_binding(std::unique_ptr<CameraKeyBinding>(new CameraKeyBinding{
        .tpe = camera_cycle_type_from_string(args.arguments.at<std::string>(KnownArgs::tpe)),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            "" }
    }));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "camera_key_binding",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateCameraKeyBinding(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
