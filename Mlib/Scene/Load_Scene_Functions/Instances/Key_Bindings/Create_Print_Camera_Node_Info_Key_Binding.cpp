#include "Create_Print_Camera_Node_Info_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Key_Bindings/Print_Node_Info_Key_Binding.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(seat);
}

CreatePrintCameraNodeInfoKeyBinding::CreatePrintCameraNodeInfoKeyBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreatePrintCameraNodeInfoKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto& kb = key_bindings.add_print_node_info_key_binding(std::unique_ptr<PrintNodeInfoKeyBinding>(new PrintNodeInfoKeyBinding{
        .dynamic_node = [&scene=scene, &sc=selected_cameras]() -> DanglingBaseClassPtr<SceneNode> {
            auto name = sc.camera_node_name();
            return scene.get_node(name, DP_LOC).ptr();
        },
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat)},
        .geographic_mapping = scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world"}),
        .on_destroy_key_bindings{ DestructionFunctionsRemovalTokens{ key_bindings.on_destroy, CURRENT_SOURCE_LOCATION } }}));
    kb.on_destroy_key_bindings.add([&kbs=key_bindings, &kb]() {
        kbs.delete_print_node_info_key_binding(kb);
    }, CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_print_camera_node_info_key_binding",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreatePrintCameraNodeInfoKeyBinding(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
