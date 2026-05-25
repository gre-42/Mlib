#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Invisible_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(aggregate_mode);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "invisible_resource",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                // Using loader so "write_loaded_resources" works as expected.
                RenderingContextStack::primary_scene_node_resources().add_resource_loader(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
                    [aggregate_mode=aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::aggregate_mode))]()
                    {
                        return std::make_shared<InvisibleResource>(aggregate_mode);
                    });
            });
    }
} obj;

}
