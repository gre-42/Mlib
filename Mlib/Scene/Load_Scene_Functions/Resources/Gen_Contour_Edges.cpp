#include "Gen_Contour_Edges.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SOURCE_NAME);
DECLARE_OPTION(DEST_NAME);
DECLARE_OPTION(REQUIRED_TAGS);

const std::string GenContourEdges::key = "gen_contour_edges";

LoadSceneUserFunction GenContourEdges::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^source_name=([\\w+-.]+)"
        "\\s+dest_name=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void GenContourEdges::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.generate_contour_edges(
        match[SOURCE_NAME].str(),
        match[DEST_NAME].str());
}
