#include "Add_Ac_Loader.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Ac_Level.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

const std::string AddAcLoader::key = "add_ac_loader";

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(script);
}

LoadSceneJsonUserFunction AddAcLoader::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto id_val = args.arguments.at<std::string>(KnownArgs::id);
    if (!args.asset_references.contains(id_val)) {
        args.asset_references.add(id_val);
    }
    auto& group = args.asset_references[id_val];
    group.add_asset_loader(std::make_unique<LoadAcLevel>(
        args.arguments.spath(KnownArgs::script)));
};
