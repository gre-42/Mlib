#include "Clear_Selection_Ids.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(except);
}

const std::string ClearSelectionIds::key = "clear_selection_ids";

LoadSceneJsonUserFunction ClearSelectionIds::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ClearSelectionIds::execute(args);
};

void ClearSelectionIds::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::erase_if(
        args.ui_focus.selection_ids,
        [except=args.arguments.at<std::string>(KnownArgs::except)]
        (const auto& it)
        {return it.first != except;});
}
