#include "Hud_Image.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Image_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

LoadSceneUserFunction HudImage::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*hud_image"
        "\\s+name=([\\w+-.]+)"
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& camera_node = scene.get_node(match[2].str());
    auto hud_image = std::make_shared<HudImageLogic>(
        camera_node,
        physics_engine.advance_times_,
        args.fpath(match[3].str()).path,
        resource_update_cycle_from_string(match[4].str()),
        FixedArray<float, 2>{
            safe_stof(match[5].str()),
            safe_stof(match[6].str())},
        FixedArray<float, 2>{
            safe_stof(match[7].str()),
            safe_stof(match[8].str())});
    render_logics.append(&camera_node, hud_image);
    camera_node.add_renderable(match[1].str(), hud_image);
    physics_engine.advance_times_.add_advance_time(hud_image);

}
