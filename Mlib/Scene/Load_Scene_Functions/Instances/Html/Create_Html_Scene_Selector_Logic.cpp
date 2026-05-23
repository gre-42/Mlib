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
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(on_change);
DECLARE_ARGUMENT(assets);
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(appearance);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "html_scene_selector",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                std::list<ReplacementParameterAndFilename> scene_entries;
                for (const auto& [_, rpe] : args.asset_references[args.arguments.at<std::string>(KnownArgs::assets)]) {
                    scene_entries.emplace_back(rpe);
                }
                if (scene_entries.empty()) {
                    throw std::runtime_error("Could not find a single scene file");
                }
                scene_entries.sort();
                args.index_html.add_list(
                    args.arguments.at<std::string>(KnownArgs::title),
                    std::vector(scene_entries.begin(), scene_entries.end()),
                    args.macro_line_executor);
            });
    }
} obj;

}
