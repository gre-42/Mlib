#include "Ypln_Set_Bullet_Velocity.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(VELOCITY);

LoadSceneUserFunction YplnSetBulletVelocity::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*ypln_set_bullet_velocity"
        "\\s+node=([\\w+-.]+)"
        "\\s+velocity=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        YplnSetBulletVelocity(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

YplnSetBulletVelocity::YplnSetBulletVelocity(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void YplnSetBulletVelocity::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& ypln_node = scene.get_node(match[NODE].str());
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&ypln_node.get_relative_movable());
    if (ypln == nullptr) {
        throw std::runtime_error("Relative movable is not a ypln");
    }
    ypln->set_bullet_velocity(safe_stof(match[VELOCITY].str()) * meters / s);
}
