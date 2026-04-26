#include "Save_To_Obj_File.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(prefix);
}

SaveToObjFile::SaveToObjFile(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SaveToObjFile::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    scene_node_resources.save_to_obj_file(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
        args.arguments.at<std::string>(KnownArgs::prefix),
        rvalue_address(node->absolute_model_matrix()));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "save_to_obj_file",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto node = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::node);
                if (node.has_value()) {
                    SaveToObjFile(args.physics_scene()).execute(args);
                } else {
                    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
                    scene_node_resources.save_to_obj_file(
                        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
                        args.arguments.at<std::string>(KnownArgs::prefix),
                        nullptr);
                }
            });
    }
} obj;

}
