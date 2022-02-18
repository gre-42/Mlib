#include "Set_Skater_Style_Updater.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Animation/Skater_Animation_Updater.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(RESOURCE);

LoadSceneUserFunction SetSkaterStyleUpdater::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_skater_style_updater"
        "\\s+node=([\\w+-.]*)"
        "\\s+resource=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetSkaterStyleUpdater(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetSkaterStyleUpdater::SetSkaterStyleUpdater(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSkaterStyleUpdater::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    std::string resource = match[RESOURCE].str();
    auto rb = dynamic_cast<RigidBodyVehicle*>(node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Styled node movable is not a rigid body");
    }
    if (rb->style_updater_ != nullptr) {
        throw std::runtime_error("Rigid body already has a style updater");
    }
    auto updater = std::make_unique<SkaterAnimationUpdater>(*rb, resource);
    StyleUpdater* ptr = updater.get();
    node.set_style_updater(std::move(updater));
    rb->style_updater_ = ptr;
}
