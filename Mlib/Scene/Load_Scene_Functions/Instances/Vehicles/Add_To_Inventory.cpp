#include "Add_To_Inventory.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(INVENTORY_NODE);
DECLARE_OPTION(ITEM_TYPE);
DECLARE_OPTION(AMOUNT);

const std::string AddToInventory::key = "add_to_inventory";

LoadSceneUserFunction AddToInventory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^inventory_node=([\\w+-.]+)"
        "\\s+item_type=([\\w+-.]+)"
        "\\s+amount=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    AddToInventory(args.renderable_scene()).execute(match, args);
};

AddToInventory::AddToInventory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddToInventory::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto* rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(match[INVENTORY_NODE].str()).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body vehicle");
    }
    rb->inventory_.add(
        match[ITEM_TYPE].str(),
        safe_stox<uint32_t>(match[AMOUNT].str(), "amount"));
}
