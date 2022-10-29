#include "Append_Externals_Deleter.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(NODE);

LoadSceneUserFunction AppendExternalsDeleter::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*append_externals_deleter"
        "\\s+player=([\\w+-.]+)"
        "\\s+node=([\\s\\S]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        AppendExternalsDeleter(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

AppendExternalsDeleter::AppendExternalsDeleter(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AppendExternalsDeleter::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto node_name = match[NODE].str();
    auto& node = scene.get_node(node_name);
    players.get_player(match[PLAYER].str()).append_delete_externals(
        &node,
        [&scene = scene, node_name]()
        {
            try {
                scene.delete_node(node_name);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Could not delete node \"" + node_name + "\": " + e.what());
            }
        }
    );
}
