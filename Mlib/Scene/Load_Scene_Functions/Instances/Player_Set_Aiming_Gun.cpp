#include "Player_Set_Aiming_Gun.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(YAW_NODE);
DECLARE_OPTION(GUN_NODE);

LoadSceneUserFunction PlayerSetAimingGun::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_set_aiming_gun"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+yaw_node=([\\w+-.]+)"
        "(?:\\s+gun_node=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetAimingGun(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetAimingGun::PlayerSetAimingGun(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetAimingGun::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto ypln_node = scene.get_node(match[YAW_NODE].str());
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(ypln_node->get_relative_movable());
    if (ypln == nullptr) {
        throw std::runtime_error("Relative movable is not a ypln");
    }
    SceneNode* gun_node = nullptr;
    if (match[GUN_NODE].matched) {
        gun_node = scene.get_node(match[GUN_NODE].str());
    }
    players.get_player(match[PLAYER_NAME].str()).set_ypln(*ypln, gun_node);
}
