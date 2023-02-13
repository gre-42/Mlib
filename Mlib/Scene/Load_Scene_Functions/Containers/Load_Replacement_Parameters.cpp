#include "Load_Replacement_Parameters.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(DIRECTORY);

LoadSceneUserFunction LoadReplacementParameters::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*load_replacement_parameters"
        "\\s+id=([\\w+-.]+)"
        "\\s+directory=(.+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void LoadReplacementParameters::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string id = match[ID].str();
    args.asset_references.add_replacement_parameter_group(id);
    static DECLARE_REGEX(manifest_regex, "^.*manifest.*\\.json$");
    for (const auto& root : args.fpathes(match[DIRECTORY].str())) {
        for (auto const& level_dir : list_dir(root)) {
            for (const auto& candidate_file : list_dir(level_dir)) {
                if (!Mlib::re::regex_match(candidate_file.path().filename().string(), manifest_regex)) {
                    continue;
                }
                auto path_string = candidate_file.path().string();
                try {
                    args.asset_references.add_replacement_parameter(
                        id,
                        path_string,
                        args.macro_line_executor,
                        args.rsc);
                } catch (const std::runtime_error& e) {
                    throw std::runtime_error("Error processing replacement parameter file \"" + path_string + "\": " + e.what());
                }
            }
        }
    }
    args.asset_references.sort_replacement_parameters(id);
}
