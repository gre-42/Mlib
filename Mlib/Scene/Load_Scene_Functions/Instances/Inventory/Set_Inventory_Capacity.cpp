#include "Set_Inventory_Capacity.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(inventory_node);
DECLARE_ARGUMENT(item_type);
DECLARE_ARGUMENT(capacity);
}

const std::string SetInventoryCapacity::key = "set_inventory_capacity";

LoadSceneJsonUserFunction SetInventoryCapacity::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetInventoryCapacity(args.renderable_scene()).execute(args);
};

SetInventoryCapacity::SetInventoryCapacity(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetInventoryCapacity::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::inventory_node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    rb.inventory_.set_capacity(
        args.arguments.at<std::string>(KnownArgs::item_type),
        args.arguments.at<uint32_t>(KnownArgs::capacity));
}
