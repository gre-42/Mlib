#include "Set_Skater_Style_Updater.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Animation/Skater_Animation_Updater.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SKATER_NODE);
DECLARE_OPTION(SKATEBOARD_NODE);
DECLARE_OPTION(RESOURCE);

const std::string SetSkaterStyleUpdater::key = "set_skater_style_updater";

LoadSceneUserFunction SetSkaterStyleUpdater::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^skater_node=([\\w+-.]*)"
        "\\s+skateboard_node=([\\w+-.]*)"
        "\\s+resource=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetSkaterStyleUpdater(args.renderable_scene()).execute(match, args);
};

SetSkaterStyleUpdater::SetSkaterStyleUpdater(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSkaterStyleUpdater::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& skater_node = scene.get_node(match[SKATER_NODE].str());
    auto& skateboard_node = scene.get_node(match[SKATEBOARD_NODE].str());
    std::string resource = match[RESOURCE].str();
    auto rb = dynamic_cast<RigidBodyVehicle*>(&skater_node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Styled node movable is not a rigid body");
    }
    if (rb->animation_state_updater_ != nullptr) {
        THROW_OR_ABORT("Rigid body already has a style updater");
    }
    auto updater = std::make_unique<SkaterAnimationUpdater>(*rb, skateboard_node, resource);
    AnimationStateUpdater* ptr = updater.get();
    skater_node.set_animation_state_updater(std::move(updater));
    rb->animation_state_updater_ = ptr;
}
