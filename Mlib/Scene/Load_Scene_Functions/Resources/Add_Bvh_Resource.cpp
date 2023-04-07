#include "Add_Bvh_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Load_Bvh.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Resources/Bvh_File_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

const std::string AddBvhResource::key = "add_bvh_resource";

BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(smooth_radius);
DECLARE_ARGUMENT(smooth_alpha);
DECLARE_ARGUMENT(periodic);

LoadSceneJsonUserFunction AddBvhResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(options);
    BvhConfig cfg = blender_bvh_config;
    cfg.smooth_radius = args.arguments.get<size_t>(smooth_radius);
    cfg.smooth_alpha = args.arguments.get<float>(smooth_alpha);
    cfg.periodic = args.arguments.get<bool>(periodic);
    args.scene_node_resources.add_resource_loader(
        args.arguments.get<std::string>(name),
        [filename=args.arguments.path(filename), cfg](){
            return std::make_shared<BvhFileResource>(
                filename,
                cfg);});
};
