#include "Set_Avatar_Style_Updater.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Animation/Avatar_Animation_Updater.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(AVATAR_NODE);
DECLARE_OPTION(GUN_NODE);
DECLARE_OPTION(RESOURCE_WO_GUN);
DECLARE_OPTION(RESOURCE_W_GUN);

LoadSceneUserFunction SetAvatarStyleUpdater::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_avatar_style_updater"
        "\\s+avatar_node=([\\w+-.]*)"
        "\\s+gun_node=([\\w+-.]+)"
        "\\s+resource_wo_gun=([\\w+-.]+)"
        "\\s+resource_w_gun=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetAvatarStyleUpdater(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetAvatarStyleUpdater::SetAvatarStyleUpdater(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetAvatarStyleUpdater::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& avatar_node = scene.get_node(match[AVATAR_NODE].str());
    auto& gun_node = scene.get_node(match[GUN_NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&avatar_node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Styled node movable is not a rigid body");
    }
    if (rb->animation_state_updater_ != nullptr) {
        throw std::runtime_error("Rigid body already has a style updater");
    }
    auto updater = std::make_unique<AvatarAnimationUpdater>(
        *rb,
        gun_node,
        match[RESOURCE_WO_GUN].str(),
        match[RESOURCE_W_GUN].str());
    AnimationStateUpdater* ptr = updater.get();
    avatar_node.set_animation_state_updater(std::move(updater));
    rb->animation_state_updater_ = ptr;
}
