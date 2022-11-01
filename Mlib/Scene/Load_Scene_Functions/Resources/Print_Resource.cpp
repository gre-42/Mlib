#include "Print_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);

LoadSceneUserFunction PrintResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*print_resource"
        "\\s+name=([\\w+-.]+)");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void PrintResource::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.print(match[NAME].str(), std::cerr);
}
