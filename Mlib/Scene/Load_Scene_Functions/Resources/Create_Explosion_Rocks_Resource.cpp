#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Scene_Node_Resources/Explosion_Rocks_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(nrocks);
DECLARE_ARGUMENT(rocks);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "explosion_rocks",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                auto& sr = RenderingContextStack::primary_scene_node_resources();
                sr.add_resource_loader(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
                    [
                        nrocks = args.arguments.at<size_t>(KnownArgs::nrocks),
                        rocks = args.arguments.at<std::vector<ExplosionRockDescriptor>>(KnownArgs::rocks),
                        &sr = sr,
                        &bp = args.bullet_property_db
                    ](){
                        return std::make_shared<ExplosionRocksResource>(
                            sr,
                            bp,
                            rocks,
                            nrocks);});
            });
    }
} obj;

}
