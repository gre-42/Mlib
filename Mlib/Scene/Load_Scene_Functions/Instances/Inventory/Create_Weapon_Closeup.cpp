#include "Create_Weapon_Closeup.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(cycle_node);
}

const std::string CreateWeaponCloseup::key = "create_weapon_closeup";

LoadSceneJsonUserFunction CreateWeaponCloseup::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateWeaponCloseup(args.renderable_scene()).execute(args);
};

CreateWeaponCloseup::CreateWeaponCloseup(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponCloseup::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& cycle_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::cycle_node));
    auto wc = dynamic_cast<WeaponCycle*>(&cycle_node.get_node_modifier());
    if (wc == nullptr) {
        THROW_OR_ABORT("Node modifier is not a weapon inventory");
    }
    wc->create_weapon_closeup();
}
