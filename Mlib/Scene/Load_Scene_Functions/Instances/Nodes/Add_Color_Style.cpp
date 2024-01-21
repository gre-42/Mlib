#include "Add_Color_Style.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(selector);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(ambient);
DECLARE_ARGUMENT(diffuse);
DECLARE_ARGUMENT(specular);
DECLARE_ARGUMENT(reflection_strength);
DECLARE_ARGUMENT(reflection_maps);
}

const std::string AddColorStyle::key = "add_color_style";

LoadSceneJsonUserFunction AddColorStyle::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    AddColorStyle(args.renderable_scene()).execute(args);
};

AddColorStyle::AddColorStyle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddColorStyle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::map<std::string, std::string> parsed_reflection_maps;
    if (args.arguments.contains(KnownArgs::reflection_maps)) {
        parsed_reflection_maps = args.arguments.at<std::map<std::string, std::string>>(KnownArgs::reflection_maps);
    }
    auto style = std::unique_ptr<ColorStyle>(new ColorStyle{
        .selector = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::selector, "")),
        .ambient = args.arguments.contains(KnownArgs::ambient)
            ? args.arguments.at<FixedArray<float, 3>>(KnownArgs::ambient)
            : fixed_full<float, 3>(-1),
        .diffuse = args.arguments.contains(KnownArgs::diffuse)
            ? args.arguments.at<FixedArray<float, 3>>(KnownArgs::diffuse)
            : fixed_full<float, 3>(-1),
        .specular = args.arguments.contains(KnownArgs::specular)
            ? args.arguments.at<FixedArray<float, 3>>(KnownArgs::specular)
            : fixed_full<float, 3>(-1),
        .reflection_maps = std::move(parsed_reflection_maps),
        .reflection_strength = args.arguments.at<float>(KnownArgs::reflection_strength, -1.f)});
    if (args.arguments.contains(KnownArgs::node)) {
        DanglingRef<SceneNode> n = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
        n->add_color_style(std::move(style));
    } else {
        scene.add_color_style(std::move(style));
    }
}
