#include "Set_Background_Color.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color);
}

const std::string SetBackgroundColor::key = "set_background_color";

LoadSceneJsonUserFunction SetBackgroundColor::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetBackgroundColor(args.renderable_scene()).execute(args);
};

SetBackgroundColor::SetBackgroundColor(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetBackgroundColor::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto background_color = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::color);
    standard_render_logic.set_background_color(background_color);
    post_processing_logic.set_background_color(background_color);
}
