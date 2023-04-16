#include "Add_Weapon_To_Cycle.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(CYCLE_NODE);
DECLARE_OPTION(ENTRY_NAME);
DECLARE_OPTION(AMMO_TYPE);
DECLARE_OPTION(COOL_DOWN);
DECLARE_OPTION(BULLET_DAMAGE);
DECLARE_OPTION(BULLET_DAMAGE_RADIUS);
DECLARE_OPTION(BULLET_VELOCITY);
DECLARE_OPTION(BULLET_FEELS_GRAVITY);
DECLARE_OPTION(RANGE_MIN);
DECLARE_OPTION(RANGE_MAX);
DECLARE_OPTION(CREATE);

const std::string AddWeaponToInventory::key = "add_weapon_to_cycle";

LoadSceneUserFunction AddWeaponToInventory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^cycle_node=([\\w+-.]+)"
        "\\s+entry_name=([\\w+-. \\(\\)/]+)"
        "\\s+ammo_type=(\\w+)"
        "\\s+cool_down=([\\w+-.]+)"
        "\\s+bullet_damage=([\\w+-.]+)"
        "\\s+bullet_damage_radius=([\\w+-.]+)"
        "\\s+bullet_velocity=([\\w+-.]+)"
        "\\s+bullet_feels_gravity=(0|1)"
        "\\s+range_min=([\\w+-.]+)"
        "\\s+range_max=([\\w+-.]+)"
        "\\s+create=([\\s\\S]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    AddWeaponToInventory(args.renderable_scene()).execute(match, args);
};

AddWeaponToInventory::AddWeaponToInventory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddWeaponToInventory::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& cycle_node = scene.get_node(match[CYCLE_NODE].str());
    std::string entry_name = match[ENTRY_NAME].str();
    std::string create = match[CREATE].str();
    WeaponCycle* wc = dynamic_cast<WeaponCycle*>(&cycle_node.get_node_modifier());
    if (wc == nullptr) {
        THROW_OR_ABORT("Node modifier is not a weapon inventory");
    }
    std::string ammo_type = match[AMMO_TYPE].str();
    float cool_down = safe_stof(match[COOL_DOWN].str());
    float bullet_damage = safe_stof(match[BULLET_DAMAGE].str());
    float bullet_damage_radius = safe_stof(match[BULLET_DAMAGE_RADIUS].str());
    float bullet_velocity = safe_stof(match[BULLET_VELOCITY].str());
    bool bullet_feels_gravity = safe_stob(match[BULLET_FEELS_GRAVITY].str());
    wc->add_weapon(
        entry_name,
        WeaponInfo{
            .create_weapon = [
                macro_line_executor = args.macro_line_executor,
                create,
                ammo_type,
                cool_down,
                bullet_damage,
                bullet_damage_radius,
                bullet_velocity,
                bullet_feels_gravity]()
            {
                SubstitutionMap subst;
                subst.insert("AMMO_TYPE", ammo_type);
                subst.insert("COOL_DOWN", std::to_string(cool_down));
                subst.insert("BULLET_DAMAGE", std::to_string(bullet_damage));
                subst.insert("BULLET_DAMAGE_RADIUS", std::to_string(bullet_damage_radius));
                subst.insert("BULLET_VELOCITY", std::to_string(bullet_velocity));
                subst.insert("BULLET_FEELS_GRAVITY", bullet_feels_gravity ? "true" : "false");
                macro_line_executor(create, &subst);
            },
            .ammo_type = ammo_type,
            .cool_down = cool_down * s,
            .bullet_damage = bullet_damage,
            .bullet_damage_radius = bullet_damage_radius * meters,
            .bullet_velocity = bullet_velocity * meters / s,
            .bullet_feels_gravity = bullet_feels_gravity,
            .range_min = safe_stof(match[RANGE_MIN].str()) * meters,
            .range_max = safe_stof(match[RANGE_MAX].str()) * meters});
}
