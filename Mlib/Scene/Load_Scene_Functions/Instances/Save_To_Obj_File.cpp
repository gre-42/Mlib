#include "Save_To_Obj_File.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
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

const std::string SaveToObjFile::key = "save_to_obj_file";

LoadSceneJsonUserFunction SaveToObjFile::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SaveToObjFile(args.renderable_scene()).execute(args);
};

SaveToObjFile::SaveToObjFile(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SaveToObjFile::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto node = args.arguments.try_at<std::string>(KnownArgs::node);
    scene_node_resources.save_to_obj_file(
        args.arguments.at<std::string>(KnownArgs::resource),
        args.arguments.at<std::string>(KnownArgs::prefix),
        node.has_value()
            ? rvalue_address(scene.get_node(*node, DP_LOC)->absolute_model_matrix())
            : nullptr);
}
