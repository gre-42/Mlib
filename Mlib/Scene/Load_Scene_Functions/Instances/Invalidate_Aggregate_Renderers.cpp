#include "Invalidate_Aggregate_Renderers.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Aggregate_Render_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

const std::string InvalidateAggregateRenderers::key = "invalidate_aggregate_renderers";

LoadSceneJsonUserFunction InvalidateAggregateRenderers::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate({});
    InvalidateAggregateRenderers(args.renderable_scene()).execute(args);
};

InvalidateAggregateRenderers::InvalidateAggregateRenderers(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void InvalidateAggregateRenderers::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    renderable_scene.wait_until_done();
    aggregate_render_logic.invalidate_aggregate_renderers();
}
