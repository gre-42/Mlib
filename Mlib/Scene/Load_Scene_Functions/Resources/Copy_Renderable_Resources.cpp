#include "Copy_Renderable_Resources.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SOURCE_NAME);
DECLARE_OPTION(DEST_NAME);
DECLARE_OPTION(EXCLUDE);

LoadSceneUserFunction CopyRenderableResources::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*copy_renderable_resources"
        "\\s+source_name=([\\w+-.]+)"
        "\\s+dest_name=([\\w+-.]+)"
        "\\s+exclude=(.*?)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CopyRenderableResources::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.copy_renderable_resources(
        match[SOURCE_NAME].str(),
        match[DEST_NAME].str(),
        RenderableResourceFilter{
            .exclude = Mlib::compile_regex(match[EXCLUDE].str())
        });
}
