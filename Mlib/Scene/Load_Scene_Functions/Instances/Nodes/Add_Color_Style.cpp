#include "Add_Color_Style.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SELECTOR);
DECLARE_OPTION(NODE);
DECLARE_OPTION(AMBIENCE_R);
DECLARE_OPTION(AMBIENCE_G);
DECLARE_OPTION(AMBIENCE_B);
DECLARE_OPTION(DIFFUSIVITY_R);
DECLARE_OPTION(DIFFUSIVITY_G);
DECLARE_OPTION(DIFFUSIVITY_B);
DECLARE_OPTION(SPECULARITY_R);
DECLARE_OPTION(SPECULARITY_G);
DECLARE_OPTION(SPECULARITY_B);
DECLARE_OPTION(REFLECTION_STRENGTH);
DECLARE_OPTION(REFLECTION_MAPS);

LoadSceneUserFunction AddColorStyle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_color_style"
        "(?:\\s+selector=([^\\r\\n]*)\\r?\\n)?"
        "(?:\\s*node=([\\w+-.]+))?"
        "(?:\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+reflection_strength=([\\w+-.]+))?"
        "(?:\\s+reflection_maps=((?:\\s*key=[\\w+-.]+\\s+value=[\\w+-.]+)*))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        AddColorStyle(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

AddColorStyle::AddColorStyle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddColorStyle::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::map<std::string, std::string> reflection_maps;
    if (match[REFLECTION_MAPS].matched) {
        static const DECLARE_REGEX(reflection_maps_regex, "(?:\\s*key=([\\w+-.]+)\\s+value=([\\w+-.]+)|([\\s\\S]+))");
        find_all(match[REFLECTION_MAPS].str(), reflection_maps_regex, [&reflection_maps](const Mlib::re::smatch& match2) {
            if (match2[3].matched) {
                throw std::runtime_error("Unknown element: \"" + match2[3].str() + '"');
            }
            if (!reflection_maps.insert({match2[1].str(), match2[2].str()}).second) {
                throw std::runtime_error("Duplicate reflection map key: \"" + match2[1].str());
            }
        });
    }
    auto style = std::unique_ptr<ColorStyle>(new ColorStyle{
        .selector = Mlib::compile_regex(match[SELECTOR].str()),
        .ambience = {
            match[AMBIENCE_R].matched ? safe_stof(match[AMBIENCE_R].str()) : -1,
            match[AMBIENCE_G].matched ? safe_stof(match[AMBIENCE_G].str()) : -1,
            match[AMBIENCE_B].matched ? safe_stof(match[AMBIENCE_B].str()) : -1},
        .diffusivity = {
            match[DIFFUSIVITY_R].matched ? safe_stof(match[DIFFUSIVITY_R].str()) : -1,
            match[DIFFUSIVITY_G].matched ? safe_stof(match[DIFFUSIVITY_G].str()) : -1,
            match[DIFFUSIVITY_B].matched ? safe_stof(match[DIFFUSIVITY_B].str()) : -1},
        .specularity = {
            match[SPECULARITY_R].matched ? safe_stof(match[SPECULARITY_R].str()) : -1,
            match[SPECULARITY_G].matched ? safe_stof(match[SPECULARITY_G].str()) : -1,
            match[SPECULARITY_B].matched ? safe_stof(match[SPECULARITY_B].str()) : -1},
        .reflection_maps = std::move(reflection_maps),
        .reflection_strength = match[REFLECTION_STRENGTH].matched ? safe_stof(match[REFLECTION_STRENGTH].str()) : -1});
    if (match[NODE].matched) {
        auto& node = scene.get_node(match[NODE].str());
        node.add_color_style(std::move(style));
    } else {
        scene.add_color_style(std::move(style));
    }
}
