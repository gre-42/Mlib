#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(directory);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "load_replacement_parameters",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto id_val = args.arguments.at<std::string>(KnownArgs::id);
                if (!args.asset_references.contains(id_val)) {
                    args.asset_references.add(id_val);
                }
                auto& group = args.asset_references[id_val];
                static DECLARE_REGEX(manifest_regex, "^.*manifest.*\\.json$");
                for (const auto& root : args.arguments.path_list(KnownArgs::directory)) {
                    for (const auto& level_dir : list_dir(root)) {
                        if (!is_listable(level_dir)) {
                            continue;
                        }
                        for (const auto& candidate_file : list_dir(Utf8Path::from_path(level_dir.path()))) {
                            if (!Mlib::re::regex_match(candidate_file.path().filename().string(), manifest_regex)) {
                                continue;
                            }
                            auto path_string = candidate_file.path().string();
                            try {
                                group.insert_if_active(path_string, args.macro_line_executor);
                            } catch (const std::runtime_error& e) {
                                throw std::runtime_error("Error processing replacement parameter file \"" + path_string + "\": " + e.what());
                            }
                        }
                    }
                }
            });
    }
} obj;

}
