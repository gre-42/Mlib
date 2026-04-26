#include "Add_To_Inventory.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(inventory_node);
DECLARE_ARGUMENT(item_type);
DECLARE_ARGUMENT(amount);
}

AddToInventory::AddToInventory(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void AddToInventory::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::inventory_node), CURRENT_SOURCE_LOCATION);
    auto rb = get_rigid_body_vehicle(node.get(), CURRENT_SOURCE_LOCATION);
    rb->inventory_.add(
        args.arguments.at<InventoryItem>(KnownArgs::item_type),
        args.arguments.at<uint32_t>(KnownArgs::amount));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_to_inventory",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                AddToInventory{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
