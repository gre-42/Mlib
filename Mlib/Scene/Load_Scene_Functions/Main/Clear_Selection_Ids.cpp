#include "Clear_Selection_Ids.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(EXCEPT);

const std::string ClearSelectionIds::key = "clear_selection_ids";

LoadSceneUserFunction ClearSelectionIds::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^except=(\\w+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ClearSelectionIds::execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ClearSelectionIds::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::erase_if(
        args.ui_focus.selection_ids,
        [&match](const auto& it){return it.first != match[EXCEPT].str();});
}
