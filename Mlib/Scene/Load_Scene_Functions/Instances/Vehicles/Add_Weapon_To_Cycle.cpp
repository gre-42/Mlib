#include "Add_Weapon_To_Cycle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(cycle_node);
DECLARE_ARGUMENT(entry_name);
DECLARE_ARGUMENT(ammo_type);
DECLARE_ARGUMENT(cool_down);
DECLARE_ARGUMENT(bullet_damage);
DECLARE_ARGUMENT(bullet_damage_radius);
DECLARE_ARGUMENT(bullet_velocity);
DECLARE_ARGUMENT(bullet_feels_gravity);
DECLARE_ARGUMENT(range_min);
DECLARE_ARGUMENT(range_max);
DECLARE_ARGUMENT(create_weapon);
DECLARE_ARGUMENT(create_closeup);
DECLARE_ARGUMENT(capture);
}

const std::string AddWeaponToInventory::key = "add_weapon_to_cycle";

LoadSceneJsonUserFunction AddWeaponToInventory::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    AddWeaponToInventory(args.renderable_scene()).execute(args);
};

AddWeaponToInventory::AddWeaponToInventory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddWeaponToInventory::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& cycle_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::cycle_node));
    std::string entry_name = args.arguments.at<std::string>(KnownArgs::entry_name);
    auto create_weapon = args.arguments.at(KnownArgs::create_weapon);
    auto create_closeup = args.arguments.at(KnownArgs::create_closeup);
    auto capture = args.arguments.at(KnownArgs::capture);
    auto wc = dynamic_cast<WeaponCycle*>(&cycle_node.get_node_modifier());
    if (wc == nullptr) {
        THROW_OR_ABORT("Node modifier is not a weapon inventory");
    }
    std::string ammo_type = args.arguments.at<std::string>(KnownArgs::ammo_type);
    float cool_down = args.arguments.at<float>(KnownArgs::cool_down);
    float bullet_damage = args.arguments.at<float>(KnownArgs::bullet_damage);
    float bullet_damage_radius = args.arguments.at<float>(KnownArgs::bullet_damage_radius);
    float bullet_velocity = args.arguments.at<float>(KnownArgs::bullet_velocity);
    bool bullet_feels_gravity = args.arguments.at<bool>(KnownArgs::bullet_feels_gravity);
    wc->add_weapon(
        entry_name,
        WeaponInfo{
            .create_weapon = [
                    macro_line_executor = args.macro_line_executor,
                    create_weapon,
                    capture,
                    ammo_type,
                    cool_down,
                    bullet_damage,
                    bullet_damage_radius,
                    bullet_velocity,
                    bullet_feels_gravity]()
                {
                    JsonMacroArguments subst{capture};
                    subst.insert_json("AMMO_TYPE", ammo_type);
                    subst.insert_json("COOL_DOWN", cool_down);
                    subst.insert_json("BULLET_DAMAGE", bullet_damage);
                    subst.insert_json("BULLET_DAMAGE_RADIUS", bullet_damage_radius);
                    subst.insert_json("BULLET_VELOCITY", bullet_velocity);
                    subst.insert_json("BULLET_FEELS_GRAVITY", bullet_feels_gravity);
                    macro_line_executor(JsonView{create_weapon}, &subst, nullptr);
                },
            .create_closeup = [
                    macro_line_executor = args.macro_line_executor,
                    create_closeup,
                    capture]()
                {
                    JsonMacroArguments subst{capture};
                    macro_line_executor(JsonView{create_closeup}, &subst, nullptr);
                },
            .ammo_type = ammo_type,
            .cool_down = cool_down * s,
            .bullet_damage = bullet_damage,
            .bullet_damage_radius = bullet_damage_radius * meters,
            .bullet_velocity = bullet_velocity * meters / s,
            .bullet_feels_gravity = bullet_feels_gravity,
            .range_min = args.arguments.at<float>(KnownArgs::range_min) * meters,
            .range_max = args.arguments.at<float>(KnownArgs::range_max) * meters});
}
