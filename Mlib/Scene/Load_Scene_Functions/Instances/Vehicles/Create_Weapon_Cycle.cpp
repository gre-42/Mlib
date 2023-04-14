#include "Create_Weapon_Cycle.hpp"
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
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

const std::string CreateWeaponCycle::key = "create_weapon_cycle";

LoadSceneUserFunction CreateWeaponCycle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^cycle_node=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateWeaponCycle(args.renderable_scene()).execute(match, args);
};

CreateWeaponCycle::CreateWeaponCycle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponCycle::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& cycle_node = scene.get_node(match[CYCLE_NODE].str());
    cycle_node.set_node_modifier(std::make_unique<WeaponCycle>());
}
