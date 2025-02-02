#include "Load_Asset_Manifests.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Interfaces/IAsset_Loader.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

const std::string LoadAssetManifests::key = "load_asset_manifests";

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(directory);
}

LoadSceneJsonUserFunction LoadAssetManifests::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto id_val = args.arguments.at<std::string>(KnownArgs::id);
    if (!args.asset_references.contains(id_val)) {
        args.asset_references.add(id_val);
    }
    auto& group = args.asset_references[id_val];
    for (const auto& root : args.arguments.path_list(KnownArgs::directory)) {
        for (const auto& loader : group.loaders()) {
            for (auto&& manifest : loader->try_load(root)) {
                group.insert(std::move(manifest));
            }
        }
    }
};
