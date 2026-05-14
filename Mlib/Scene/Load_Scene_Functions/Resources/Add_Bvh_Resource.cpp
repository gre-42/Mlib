#include <Mlib/Geometry/Mesh/Load/Load_Bvh.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Resources/Bvh_File_Resource.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(smooth_radius);
DECLARE_ARGUMENT(smooth_alpha);
DECLARE_ARGUMENT(periodic);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_bvh_resource",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                BvhConfig cfg = blender_bvh_config;
                cfg.smooth_radius = args.arguments.at<size_t>(KnownArgs::smooth_radius);
                cfg.smooth_alpha = args.arguments.at<float>(KnownArgs::smooth_alpha);
                cfg.periodic = args.arguments.at<bool>(KnownArgs::periodic);
                RenderingContextStack::primary_scene_node_resources().add_resource_loader(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
                    [filename=args.arguments.path(KnownArgs::filename), cfg](){
                        return std::make_shared<BvhFileResource>(
                            filename,
                            cfg);});
            });
    }
} obj;

}
