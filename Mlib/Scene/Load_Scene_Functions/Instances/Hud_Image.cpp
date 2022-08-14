#include "Hud_Image.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Image_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(CAMERA_NODE);
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(UPDATE);
DECLARE_OPTION(CENTER_X);
DECLARE_OPTION(CENTER_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);

LoadSceneUserFunction HudImage::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*hud_image"
        "\\s+camera_node=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+update=(once|always)"
        "\\s+center=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        HudImage(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

HudImage::HudImage(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void HudImage::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& camera_node = scene.get_node(match[CAMERA_NODE].str());
    auto hud_image = std::make_shared<HudImageLogic>(
        camera_node,
        physics_engine.advance_times_,
        args.fpath(match[FILENAME].str()).path,
        resource_update_cycle_from_string(match[UPDATE].str()),
        FixedArray<float, 2>{
            safe_stof(match[CENTER_X].str()),
            safe_stof(match[CENTER_Y].str())},
        FixedArray<float, 2>{
            safe_stof(match[SIZE_X].str()),
            safe_stof(match[SIZE_Y].str())});
    camera_node.set_node_hider(*hud_image);
    camera_node.add_destruction_observer(hud_image.get());
    render_logics.append(&camera_node, hud_image);
    physics_engine.advance_times_.add_advance_time(hud_image);

}
