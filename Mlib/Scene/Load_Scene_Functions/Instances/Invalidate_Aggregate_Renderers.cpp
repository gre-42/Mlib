#include "Invalidate_Aggregate_Renderers.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction InvalidateAggregateRenderers::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*invalidate_aggregate_renderers$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        InvalidateAggregateRenderers(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

InvalidateAggregateRenderers::InvalidateAggregateRenderers(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void InvalidateAggregateRenderers::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    standard_render_logic.invalidate_aggregate_renderers();
}
