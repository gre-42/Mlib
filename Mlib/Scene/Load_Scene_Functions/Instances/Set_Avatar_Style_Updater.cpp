#include "Set_Avatar_Style_Updater.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Animation/Avatar_Animation_Updater.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    
    auto avatar_node = scene.get_node(match[1].str());
    auto gun_node = scene.get_node(match[2].str());
    std::string resource_wo_gun = match[3].str();
    std::string resource_w_gun = match[4].str();
    auto rb = dynamic_cast<RigidBodyVehicle*>(avatar_node->get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Styled node movable is not a rigid body");
    }
    if (rb->style_updater_ != nullptr) {
        throw std::runtime_error("Rigid body already has a style updater");
    }
    auto updater = std::make_unique<AvatarAnimationUpdater>(*rb, *gun_node, resource_wo_gun, resource_w_gun);
    StyleUpdater* ptr = updater.get();
    avatar_node->set_style_updater(std::move(updater));
    rb->style_updater_ = ptr;

}
