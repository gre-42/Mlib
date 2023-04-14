#include "Create_Weapon_Cycle_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(ROLE);

DECLARE_OPTION(NODE);
DECLARE_OPTION(WEAPON_INCREMENT);

const std::string CreateWeaponCycleKeyBinding::key = "weapon_cycle_key_binding";

LoadSceneUserFunction CreateWeaponCycleKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^id=([\\w+-.]+)"
        "\\s+role=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+weapon_increment=([\\d-]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateWeaponCycleKeyBinding(args.renderable_scene()).execute(match, args);
};

CreateWeaponCycleKeyBinding::CreateWeaponCycleKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponCycleKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    key_bindings.add_weapon_inventory_key_binding(WeaponCycleKeyBinding{
        .id = match[ID].str(),
        .role = match[ROLE].str(),
        .node = &scene.get_node(match[NODE].str()),
        .direction = safe_stoi(match[WEAPON_INCREMENT].str())});
}
