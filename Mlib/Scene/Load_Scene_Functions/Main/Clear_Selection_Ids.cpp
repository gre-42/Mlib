#include "Clear_Selection_Ids.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

LoadSceneUserFunction ClearSelectionIds::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*clear_selection_ids"
        "\\s+except=(\\w+)$");
    std::smatch match;
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
        [&match](const auto& it){return it.first != match[1].str();});
}
