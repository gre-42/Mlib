#include "Invalidate_Aggregate_Renderers.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

const std::string InvalidateAggregateRenderers::key = "invalidate_aggregate_renderers";

LoadSceneUserFunction InvalidateAggregateRenderers::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    InvalidateAggregateRenderers(args.renderable_scene()).execute(match, args);
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
