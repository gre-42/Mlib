#include "Invalidate_Aggregate_Renderers.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>

using namespace Mlib;

const std::string InvalidateAggregateRenderers::key = "invalidate_aggregate_renderers";

LoadSceneJsonUserFunction InvalidateAggregateRenderers::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate({});
    InvalidateAggregateRenderers(args.renderable_scene()).execute(args);
};

InvalidateAggregateRenderers::InvalidateAggregateRenderers(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void InvalidateAggregateRenderers::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.render_set_fps.set_fps.execute([&scene=scene, &rl=standard_render_logic](){
        scene.wait_until_done();
        rl.invalidate_aggregate_renderers();
    });
}
