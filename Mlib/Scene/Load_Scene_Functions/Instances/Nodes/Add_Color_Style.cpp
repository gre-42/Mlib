#include "Add_Color_Style.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(selector);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(ambience);
DECLARE_ARGUMENT(diffusivity);
DECLARE_ARGUMENT(specularity);
DECLARE_ARGUMENT(reflection_strength);
DECLARE_ARGUMENT(reflection_maps);

const std::string AddColorStyle::key = "add_color_style";

LoadSceneUserFunction AddColorStyle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments;
    json_macro_arguments.insert_json(nlohmann::json::parse(args.line));
    json_macro_arguments.validate(options);
    AddColorStyle(args.renderable_scene()).execute(json_macro_arguments, args);
};

AddColorStyle::AddColorStyle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddColorStyle::execute(
    const JsonMacroArguments& json_macro_arguments,
    const LoadSceneUserFunctionArgs& args)
{
    std::map<std::string, std::string> parsed_reflection_maps;
    if (json_macro_arguments.contains_json(reflection_maps)) {
        parsed_reflection_maps = json_macro_arguments.at<std::map<std::string, std::string>>(reflection_maps);
    }
    auto style = std::unique_ptr<ColorStyle>(new ColorStyle{
        .selector = Mlib::compile_regex(json_macro_arguments.at<std::string>(selector, "")),
        .ambience = json_macro_arguments.contains_json(ambience)
            ? json_macro_arguments.at<FixedArray<float, 3>>(ambience)
            : fixed_full<float, 3>(-1),
        .diffusivity = json_macro_arguments.contains_json(diffusivity)
            ? json_macro_arguments.at<FixedArray<float, 3>>(diffusivity)
            : fixed_full<float, 3>(-1),
        .specularity = json_macro_arguments.contains_json(specularity)
            ? json_macro_arguments.at<FixedArray<float, 3>>(specularity)
            : fixed_full<float, 3>(-1),
        .reflection_maps = std::move(parsed_reflection_maps),
        .reflection_strength = json_macro_arguments.at<float>(reflection_strength, -1.f)});
    if (json_macro_arguments.contains_json(node)) {
        auto& n = scene.get_node(json_macro_arguments.at<std::string>(node));
        n.add_color_style(std::move(style));
    } else {
        scene.add_color_style(std::move(style));
    }
}
