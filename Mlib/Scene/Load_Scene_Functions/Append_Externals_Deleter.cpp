#include "Append_Externals_Deleter.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(MACRO);

LoadSceneInstanceFunction::UserFunction AppendExternalsDeleter::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*append_externals_deleter"
        "\\s+player=([\\w+-.]+)"
        "\\s+macro=([\\s\\S]+)$");
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
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    players.get_player(match[PLAYER].str()).append_delete_externals(
        [macro_line_executor = args.macro_line_executor,
         macro = match[MACRO].str(),
         &rsc = args.rsc]()
        {
            macro_line_executor(macro, nullptr, rsc);
        }
    );
}
