#include "Ypln_Update_Bullet_Properties.hpp"
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
DECLARE_OPTION(FEELS_GRAVITY);

LoadSceneUserFunction YplnUpdateBulletProperties::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*ypln_update_bullet_properties"
        "\\s+node=([\\w+-.]+)"
        "\\s+velocity=([\\w+-.]+)"
        "\\s+feels_gravity=(0|1)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        YplnUpdateBulletProperties(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

YplnUpdateBulletProperties::YplnUpdateBulletProperties(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void YplnUpdateBulletProperties::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& ypln_node = scene.get_node(match[NODE].str());
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&ypln_node.get_relative_movable());
    if (ypln == nullptr) {
        throw std::runtime_error("Relative movable is not a ypln");
    }
    ypln->set_bullet_velocity(safe_stof(match[VELOCITY].str()) * meters / s);
    ypln->set_bullet_feels_gravity(safe_stob(match[FEELS_GRAVITY].str()));
}
