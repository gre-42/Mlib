#include "Set_Inventory_Capacity.hpp"
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(INVENTORY_NODE);
DECLARE_OPTION(ITEM_TYPE);
DECLARE_OPTION(CAPACITY);

LoadSceneUserFunction SetInventoryCapacity::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_inventory_capacity"
        "\\s+inventory_node=([\\w+-.]+)"
        "\\s+item_type=([\\w+-.]+)"
        "\\s+capacity=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetInventoryCapacity(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetInventoryCapacity::SetInventoryCapacity(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetInventoryCapacity::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto* rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(match[INVENTORY_NODE].str()).get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body vehicle");
    }
    rb->inventory_.set_capacity(
        match[ITEM_TYPE].str(),
        safe_stox<uint32_t>(match[CAPACITY].str()));
}
