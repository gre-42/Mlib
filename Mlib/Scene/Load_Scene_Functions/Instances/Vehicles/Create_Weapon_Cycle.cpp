#include "Create_Weapon_Cycle.hpp"
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(INVENTORY_NODE);
DECLARE_OPTION(STORAGE_NODE);

LoadSceneUserFunction CreateWeaponCycle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_weapon_cycle"
        "\\s+inventory_node=([\\w+-.]+)"
        "\\s+cycle_node=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateWeaponCycle(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateWeaponCycle::CreateWeaponCycle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponCycle::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto* rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(match[INVENTORY_NODE].str()).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body vehicle");
    }
    auto& cycle_node = scene.get_node(match[STORAGE_NODE].str());
    cycle_node.set_node_modifier(std::make_unique<WeaponCycle>(rb->inventory_));
}
