#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Remote/Config_Server/Index_Http_Response_Generator.hpp>
#include <list>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(items_path);
DECLARE_ARGUMENT(selection_path);
DECLARE_ARGUMENT(assets);
DECLARE_ARGUMENT(title);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_assets_html_endpoint",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                std::list<ReplacementParameterAndFilename> entries;
                for (const auto& [_, rpe] : args.asset_references[args.arguments.at<std::string>(KnownArgs::assets)]) {
                    entries.emplace_back(rpe);
                }
                if (entries.empty()) {
                    throw std::runtime_error(
                        "Could not find a single \"" +
                        args.arguments.at<std::string>(KnownArgs::assets) +
                        "\" *_manifest.json file in path \"" +
                        args.arguments.at<std::string>(KnownArgs::items_path) + '"');
                }
                entries.sort();
                args.index_html.add_list(
                    args.arguments.at<std::string>(KnownArgs::items_path),
                    args.arguments.at<std::string>(KnownArgs::selection_path),
                    args.arguments.at<std::string>(KnownArgs::title),
                    std::vector(entries.begin(), entries.end()),
                    args.macro_line_executor);
            });
    }
} obj;

}
